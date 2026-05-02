// [CLAUDE AI WAS USED FOR THIS ENTIRE CODE FILE]

#pragma once
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <unordered_map>

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

// Start an MD5 check
void BackgroundWorker_StartMD5Check(std::string filePath);
