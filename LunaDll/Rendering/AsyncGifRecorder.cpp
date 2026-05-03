// [CLAUDE AI WAS USED FOR THIS ENTIRE FILE]

#include <iostream>
#include "AsyncGifRecorder.h"
#include "../Globals.h"
#include "../GlobalFuncs.h"

#include "../Misc/FileSystem.h"

#pragma comment(lib, "gifski.lib")

AsyncGifRecorder::AsyncGifRecorder() : 
    m_workerThread(nullptr),
    m_isEncoding(),
    m_isRunning(),
    m_StateChangeMutex(),
    m_BufferCount(),
    m_fileName(),
    m_error(false),
    m_opened(false),
    m_gifski(nullptr),      // replace m_gifWriter
    mLastTimestamp(0),
    mFirstTimestamp(0),     // new - track start time
    mFrameIndex(0)          // new - track frame number
{
    m_isEncoding.store(false, std::memory_order_relaxed);
    m_isRunning.store(false, std::memory_order_relaxed);
    m_BufferCount.store(0, std::memory_order_relaxed);
}

AsyncGifRecorder::~AsyncGifRecorder()
{
    if (m_workerThread && m_workerThread->joinable()){
        exitWorkerThread();
        m_workerThread->join();
        m_workerThread = nullptr;
    }
    // gifski handle is freed by gifski_finish, no delete needed
}

uint64_t AsyncGifRecorder::GetCurrentGIFSize()
{
    if (m_fileName.empty()) return 0;
    
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (GetFileAttributesExW(m_fileName.c_str(), GetFileExInfoStandard, &fileInfo))
    {
        ULARGE_INTEGER size;
        size.HighPart = fileInfo.nFileSizeHigh;
        size.LowPart = fileInfo.nFileSizeLow;
        return size.QuadPart;
    }
    return 0;
}

void AsyncGifRecorder::addNextFrameToProcess(int width, int height, BYTE* pData, uint32_t timestamp)
{
    std::unique_lock<std::mutex> lck(m_StateChangeMutex);
    if (!isRunning())
    {
        if (pData)
            delete[] pData;
        return;
    }

    GifRecorderCMDItem nextFrameItem;
    nextFrameItem.cmd = GIFREC_NEXTFRAME;
    nextFrameItem.width = width;
    nextFrameItem.height = height;
    nextFrameItem.timestamp = timestamp;
    nextFrameItem.data = pData;

    m_BufferCount++;
    nextFrames.push(nextFrameItem);
}

void AsyncGifRecorder::init()
{
    m_workerThread = new std::thread([this](){ workerFunc(); });
}

void AsyncGifRecorder::exitWorkerThread()
{
    GifRecorderCMDItem exitData;
    exitData.cmd = GIFREC_EXIT;
    nextFrames.push(exitData);
}

void AsyncGifRecorder::workerFunc()
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);

    while (true) {
        GifRecorderCMDItem nextData = nextFrames.pop();
        
        std::unique_ptr<BYTE[]> wrappedData;
        if (nextData.data)
            wrappedData.reset(nextData.data);

        if (nextData.cmd == GIFREC_EXIT)
            return;
        
        switch (nextData.cmd)
        {
        case GIFREC_START:
        {
            if (m_isEncoding.load(std::memory_order_relaxed))
                continue;

            std::wstring screenshotPath = gUserFilesPathWCHAR + std::wstring(L"\\!gif-recordings");
            if (GetFileAttributesW(screenshotPath.c_str()) & INVALID_FILE_ATTRIBUTES) {
                CreateDirectoryW(screenshotPath.c_str(), NULL);
            }
            screenshotPath += L"\\";
            screenshotPath += Str2WStr(generateTimestampForFilename()) + std::wstring(L".gif");
            m_fileName = screenshotPath;

            // Create gifski instance
            GifskiSettings settings;
            memset(&settings, 0, sizeof(settings));
            settings.quality = 90;
            settings.fast = false;
            settings.repeat = 0;
            m_gifski = gifski_new(&settings);

            if (!m_gifski)
            {
                m_error = true;
                m_isEncoding.store(true, std::memory_order_relaxed);
                break;
            }

            // Set output file (must be UTF-8/ASCII path)
            std::string filePathUtf8 = WStr2Str(m_fileName);
            GifskiError err = gifski_set_file_output(m_gifski, filePathUtf8.c_str());
            if (err != GIFSKI_OK)
            {
                gifski_finish(m_gifski);
                m_gifski = nullptr;
                m_error = true;
                m_isEncoding.store(true, std::memory_order_relaxed);
                break;
            }

            m_error = false;
            m_opened = true;
            mFrameIndex = 0;
            mFirstTimestamp = 0;
            mLastTimestamp = 0;

            m_isEncoding.store(true, std::memory_order_relaxed);
            break;
        }
        case GIFREC_NEXTFRAME:
        {
            if (!m_isEncoding.load(std::memory_order_relaxed))
            {
                m_BufferCount--;
                continue;
            }

            if (!m_error && m_gifski)
            {
                // Check disk space before adding next frame
                // Stop if less than 1MB remaining after accounting for current GIF size
                constexpr uint64_t MIN_FREE_SPACE = 1 * 1024 * 1024; // 1MB
                uint64_t gifSize = AsyncGifRecorder::GetCurrentGIFSize();
                
                if (!FileSystem::HasEnoughDiskSpace(gAppPathWCHAR, (int)MIN_FREE_SPACE + (int)gifSize))
                {
                    // Not enough space, stop recording
                    gifski_finish(m_gifski);
                    m_gifski = nullptr;
                    m_isRunning.store(false, std::memory_order_relaxed);
                    m_isEncoding.store(false, std::memory_order_relaxed);
                    m_error = false;
                    m_opened = false;
                    m_BufferCount--;
                    
                    // Notify Lua
                    if (gLunaLua.isValid())
                    {
                        std::shared_ptr<Event> ev = std::make_shared<Event>("onGifRecordFull", false);
                        ev->setDirectEventName("onGifRecordFull");
                        ev->setLoopable(false);
                        gLunaLua.callEvent(ev);
                    }

                    break;
                }

                // Set first timestamp as reference point
                if (mFrameIndex == 0)
                    mFirstTimestamp = nextData.timestamp;

                // Normal frame adding
                double pts = (nextData.timestamp - mFirstTimestamp) / 1000.0;
                GifskiError err = gifski_add_frame_rgba(
                    m_gifski,
                    mFrameIndex,
                    nextData.width,
                    nextData.height,
                    nextData.data,
                    pts
                );

                if (err != GIFSKI_OK)
                    m_error = true;
                else
                    mFrameIndex++;
            }
            m_BufferCount--;
            break;
        }
        case GIFREC_STOP:
        {
            if (!m_isEncoding.load(std::memory_order_relaxed))
                continue;

            if (m_gifski)
            {
                // gifski_finish blocks until all frames are written
                gifski_finish(m_gifski);
                m_gifski = nullptr;
            }

            m_opened = false;
            m_error = false;
            m_isEncoding.store(false, std::memory_order_relaxed);
            break;
        }
        default:
            break;
        }
    }
}

void AsyncGifRecorder::start()
{
    std::unique_lock<std::mutex> lck(m_StateChangeMutex);
    if (isRunning())
        return;

    GifRecorderCMDItem startCmd;
    memset(&startCmd, 0, sizeof(startCmd));
    startCmd.cmd = GIFREC_START;
    nextFrames.push(startCmd);
    m_isRunning.store(true, std::memory_order_relaxed);
}

void AsyncGifRecorder::stop()
{
    std::unique_lock<std::mutex> lck(m_StateChangeMutex);
    if (!isRunning())
        return;

    GifRecorderCMDItem stopCmd;
    memset(&stopCmd, 0, sizeof(stopCmd));
    stopCmd.cmd = GIFREC_STOP;
    nextFrames.push(stopCmd);
    m_isRunning.store(false, std::memory_order_relaxed);
}
