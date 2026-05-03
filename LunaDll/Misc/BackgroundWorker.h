// [CLAUDE AI WAS USED FOR THIS ENTIRE CODE FILE]

#pragma once
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <unordered_map>
#include <atomic>

// Store a result from the background thread
void BackgroundWorker_SetResult(std::string key, std::string value);

// Get a result from the main thread (returns "" if not ready yet)
std::string BackgroundWorker_GetResult(std::string key);

// Check if a result is ready
bool BackgroundWorker_HasResult(std::string key);

// Clear a result after reading it
void BackgroundWorker_ClearResult(std::string key);

// Clear all results
void BackgroundWorker_ClearAllResults();

// Queue a task to run on the background thread
void BackgroundWorker_Queue(std::function<void()> task);
void BackgroundWorker_Start();
void BackgroundWorker_Stop();

// Polls for any thread process
void BackgroundWorker_Poll();

// Checking if the background worker is busy
extern std::atomic<bool> gBackgroundWorkerBusy;

// The global for the main thread speed.
extern int gBackgroundWorkerSleepMicros;

// Thread speed calls.
void BackgroundWorker_SetThreadSpeed(int microseconds);
void BackgroundWorker_ResetThreadSpeed();

// -- FUNCTIONS THAT RUNS WITH THE BACKGROUNDWORKER GO BELOW

// Start an MD5 check
void BackgroundWorker_StartMD5Check(std::string filePath);
