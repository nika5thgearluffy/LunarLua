// [CLAUDE AI WAS USED FOR THIS ENTIRE CODE FILE]

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "BackgroundWorker.h"

#include "../Globals.h"
#include "../Misc/FileSystem.h"

static std::unordered_map<std::string, std::string> gResultMap;
static std::mutex gResultMutex;

void BackgroundWorker_SetResult(std::string key, std::string value)
{
    std::lock_guard<std::mutex> lock(gResultMutex);
    gResultMap[key] = value;
}

std::string BackgroundWorker_GetResult(std::string key)
{
    std::lock_guard<std::mutex> lock(gResultMutex);
    auto it = gResultMap.find(key);
    if (it != gResultMap.end())
        return it->second;
    return "";
}

bool BackgroundWorker_HasResult(std::string key)
{
    std::lock_guard<std::mutex> lock(gResultMutex);
    return gResultMap.find(key) != gResultMap.end();
}

void BackgroundWorker_ClearResult(std::string key)
{
    std::lock_guard<std::mutex> lock(gResultMutex);
    gResultMap.erase(key);
}

void BackgroundWorker_ClearAllResults()
{
    std::lock_guard<std::mutex> lock(gResultMutex);
    gResultMap.clear();
}

static std::queue<std::function<void()>> gTaskQueue;
static std::mutex gTaskMutex;
static std::condition_variable gTaskCV;
static std::atomic<bool> gBackgroundWorkerThreadTick(false);

void BackgroundWorker_Start()
{
    gBackgroundWorkerRunning = true;
    gBackgroundWorkerThread = new std::thread([]()
    {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
        while (gBackgroundWorkerRunning)
        {
            // Process any queued tasks
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(gTaskMutex);
                if (!gTaskQueue.empty())
                {
                    task = gTaskQueue.front();
                    gTaskQueue.pop();
                }
            }
            if (task)
                task();

            // Signal main thread for onThread event
            gBackgroundWorkerThreadTick.store(true);

            // Small sleep to avoid burning CPU when idle
            std::this_thread::sleep_for(std::chrono::microseconds(100));

            // Mark if the queue is busy or not for a separate LunaLua event
            std::lock_guard<std::mutex> lock(gTaskMutex);
            if (gTaskQueue.empty())
                gBackgroundWorkerBusy.store(false);
        }
    });
}

void BackgroundWorker_Stop()
{
    gBackgroundWorkerRunning = false;
    gTaskCV.notify_all();
    if (gBackgroundWorkerThread && gBackgroundWorkerThread->joinable())
    {
        gBackgroundWorkerThread->join();
        delete gBackgroundWorkerThread;
        gBackgroundWorkerThread = nullptr;
    }
}

std::atomic<bool> gBackgroundWorkerBusy(false);

void BackgroundWorker_Queue(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(gTaskMutex);
        gTaskQueue.push(task);
    }
    gBackgroundWorkerBusy.store(true);
    gTaskCV.notify_one();
}

void BackgroundWorker_Poll()
{
    if (gBackgroundWorkerThreadTick.exchange(false))
    {
        if (gLunaLua.isValid())
        {
            std::shared_ptr<Event> ev = std::make_shared<Event>("onThread", false);
            ev->setDirectEventName("onThread");
            ev->setLoopable(false);
            gLunaLua.callEvent(ev);
        }
    }

    // Fire Lua event while background worker is busy
    static bool wasWorkerBusy = false;
    bool isWorkerBusy = gBackgroundWorkerBusy.load();

    if (isWorkerBusy)
    {
        if (gLunaLua.isValid())
        {
            std::shared_ptr<Event> ev = std::make_shared<Event>("onThreadBusy", false);
            ev->setDirectEventName("onThreadBusy");
            ev->setLoopable(false);
            gLunaLua.callEvent(ev);
        }
    }
    else if (wasWorkerBusy)
    {
        // Worker just finished, fire completion event
        if (gLunaLua.isValid())
        {
            std::shared_ptr<Event> ev = std::make_shared<Event>("onThreadBusyComplete", false);
            ev->setDirectEventName("onThreadBusyComplete");
            ev->setLoopable(false);
            gLunaLua.callEvent(ev);
        }
    }

    wasWorkerBusy = isWorkerBusy;
}



// Define all background worker functions below
void BackgroundWorker_StartMD5Check(std::string filePath)
{
    BackgroundWorker_Queue([filePath]()
    {
        std::string hash = FileSystem::GetMD5Hash(filePath);
        BackgroundWorker_SetResult(filePath, hash);
    });
}
