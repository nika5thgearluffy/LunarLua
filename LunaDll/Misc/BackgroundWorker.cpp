// [CLAUDE AI WAS USED FOR THIS ENTIRE CODE FILE]

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

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

void BackgroundWorker_Start()
{
    gBackgroundWorkerRunning = true;
    gBackgroundWorkerThread = new std::thread([]()
    {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
        while (gBackgroundWorkerRunning)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(gTaskMutex);
                // Wait until there's a task or we're shutting down
                gTaskCV.wait(lock, []{ 
                    return !gTaskQueue.empty() || !gBackgroundWorkerRunning; 
                });
                if (!gBackgroundWorkerRunning && gTaskQueue.empty())
                    return;
                task = gTaskQueue.front();
                gTaskQueue.pop();
            }
            task();
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

void BackgroundWorker_Queue(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(gTaskMutex);
        gTaskQueue.push(task);
    }
    gTaskCV.notify_one();
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
