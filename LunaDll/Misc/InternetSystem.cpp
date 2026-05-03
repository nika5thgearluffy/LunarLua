#include "InternetSystem.h"

#include <windows.h>
#include <wininet.h>
#include <future>
#include <fstream>

#include "../Globals.h"

#pragma comment(lib, "wininet.lib")

// This downloads a file and parses it as a string
// [CLAUDE AI WAS USED FOR THIS PART OF THE CODE]
std::string InternetSystem::DownloadURL(std::string url)
{
    std::string result;
    bool savingToFile = (gDownloadSavePath[0] != '\0');
    std::ofstream outFile;

    if (savingToFile)
    {
        outFile.open(gDownloadSavePath, std::ios::binary);
        if (!outFile.is_open())
            savingToFile = false;
    }

    HINTERNET hInternet = InternetOpenA("LunarLua", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return result;

    HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hConnect)
    {
        InternetCloseHandle(hInternet);
        return result;
    }

    DWORD contentLength = 0;
    DWORD bufLen = sizeof(DWORD);
    HttpQueryInfo(hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
        &contentLength, &bufLen, NULL);

    char buffer[4096];
    DWORD bytesRead = 0;
    DWORD totalRead = 0;

    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
    {
        totalRead += bytesRead;

        if (savingToFile)
        {
            // Write directly to file instead of buffering in memory
            outFile.write(buffer, bytesRead);
        }
        else
        {
            result.append(buffer, bytesRead);
        }

        if (contentLength > 0)
            gDownloadProgress = (int)((totalRead * 100) / contentLength);
    }

    if (savingToFile)
        outFile.close();

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return savingToFile ? "" : result;
}

// Gets a filename from the specificed URL.
// [CLAUDE AI WAS USED FOR THIS PART OF THE CODE]
std::string InternetSystem::GetFilenameFromURL(std::string url)
{
    size_t lastSlash = url.find_last_of('/');
    if (lastSlash == std::string::npos)
        return url;
    
    // Strip query string if present (e.g. ?apikey=...)
    std::string filename = url.substr(lastSlash + 1);
    size_t queryPos = filename.find('?');
    if (queryPos != std::string::npos)
        filename = filename.substr(0, queryPos);
    
    return filename;
}

int InternetSystem::DownloadProgress()
{
    return gDownloadProgress;
}

bool InternetSystem::IsDownloading()
{
    return gDownloadPending;
}

std::string InternetSystem::DownloadFilename()
{
    return std::string(gDownloadFilename);
}

std::string InternetSystem::DownloadURL()
{
    return std::string(gDownloadURL);
}

// [CLAUDE AI WAS USED FOR THIS PART OF THE CODE]
void InternetSystem::StartDownload(std::string url, std::string savePath)
{
    if (gDownloadPending)
        return;

    gDownloadPending = true;

    strncpy_s(gDownloadFilename, sizeof(gDownloadFilename), GetFilenameFromURL(url).c_str(), _TRUNCATE);
    strncpy_s(gDownloadURL, sizeof(gDownloadURL), url.c_str(), _TRUNCATE);
    strncpy_s(gDownloadSavePath, sizeof(gDownloadSavePath), savePath.c_str(), _TRUNCATE);
    gDownloadProgress = 0;

    gDownloadFuture = std::async(std::launch::async, []() -> std::string {
        // Use globals directly instead of capturing strings
        std::string result = DownloadURL(std::string(gDownloadURL));

        if (gDownloadSavePath[0] != '\0' && !result.empty())
        {
            std::ofstream file(gDownloadSavePath);
            if (file.is_open())
            {
                file << result;
                file.close();
            }
        }

        return result;
    });

    if (gLunaLua.isValid())
    {
        std::shared_ptr<Event> downloadStart = std::make_shared<Event>("onDownloadStart", false);
        downloadStart->setDirectEventName("onDownloadStart");
        downloadStart->setLoopable(false);
        gLunaLua.callEvent(downloadStart, std::string(gDownloadURL), std::string(gDownloadFilename));
    }
}

// Call this every frame from your game tick
// [CLAUDE AI WAS USED FOR THIS PART OF THE CODE]
void InternetSystem::Poll()
{
    // Legacy single download poll - kept for compatibility
    if (gDownloadPending)
    {
        if (gDownloadFuture.valid() &&
            gDownloadFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            std::string result = gDownloadFuture.get();
            gDownloadPending = false;

            if (gLunaLua.isValid())
            {
                std::shared_ptr<Event> ev = std::make_shared<Event>("onDownloadComplete", false);
                ev->setDirectEventName("onDownloadComplete");
                ev->setLoopable(false);
                gLunaLua.callEvent(ev, result, std::string(gDownloadURL), std::string(gDownloadFilename));
            }

            gDownloadURL[0] = '\0';
            gDownloadFilename[0] = '\0';
            gDownloadSavePath[0] = '\0';
            gDownloadProgress = 0;
        }
    }

    // Multi-download poll
    std::vector<std::string> completedIDs;
    {
        std::lock_guard<std::mutex> lock(gDownloadMapMutex);
        for (auto& pair : gDownloadMap)
        {
            DownloadEntry* entry = pair.second;
            if (!entry->pending && entry->future.valid() &&
                entry->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                completedIDs.push_back(pair.first);
            }
        }
    }

    for (const std::string& id : completedIDs)
    {
        std::string result;
        std::string filename;
        {
            std::lock_guard<std::mutex> lock(gDownloadMapMutex);
            auto it = gDownloadMap.find(id);
            if (it != gDownloadMap.end())
            {
                result = it->second->future.get();
                filename = std::string(it->second->filename);
                delete it->second;
                gDownloadMap.erase(it);
            }
        }

        if (gLunaLua.isValid())
        {
            std::shared_ptr<Event> ev = std::make_shared<Event>("onDownloadComplete", false);
            ev->setDirectEventName("onDownloadComplete");
            ev->setLoopable(false);
            gLunaLua.callEvent(ev, result, id, filename);
        }
    }
}

// Returns a download ID (the URL itself works as a unique key)
std::string InternetSystem::StartDownloadID(std::string url, std::string savePath)
{
    std::lock_guard<std::mutex> lock(gDownloadMapMutex);

    // If already downloading this URL, return existing ID
    if (gDownloadMap.find(url) != gDownloadMap.end())
        return url;

    DownloadEntry* entry = new DownloadEntry();
    entry->pending = true;
    strncpy_s(entry->filename, sizeof(entry->filename),
        InternetSystem::GetFilenameFromURL(url).c_str(), _TRUNCATE);
    strncpy_s(entry->savePath, sizeof(entry->savePath),
        savePath.c_str(), _TRUNCATE);
    strncpy_s(entry->url, sizeof(entry->url),
        url.c_str(), _TRUNCATE);

    gDownloadMap[url] = entry;

    // Also update legacy globals to point to this download
    gDownloadPending = true;
    strncpy_s(gDownloadFilename, sizeof(gDownloadFilename),
        entry->filename, _TRUNCATE);
    gDownloadProgress = 0;

    entry->future = std::async(std::launch::async, [url, savePath, entry]() -> std::string
    {
        std::string result = InternetSystem::DownloadURL(url);

        if (!savePath.empty() && !result.empty())
        {
            std::ofstream file(savePath);
            if (file.is_open())
            {
                file << result;
                file.close();
            }
        }

        entry->pending = false;
        return result;
    });

    return url; // URL is the ID
}

bool InternetSystem::IsDownloadingID(std::string id)
{
    std::lock_guard<std::mutex> lock(gDownloadMapMutex);
    auto it = gDownloadMap.find(id);
    if (it == gDownloadMap.end()) return false;
    return it->second->pending;
}

int InternetSystem::GetDownloadProgressID(std::string id)
{
    std::lock_guard<std::mutex> lock(gDownloadMapMutex);
    auto it = gDownloadMap.find(id);
    if (it == gDownloadMap.end()) return 0;
    return it->second->progress.load();
}

std::string InternetSystem::GetDownloadFilenameID(std::string id)
{
    std::lock_guard<std::mutex> lock(gDownloadMapMutex);
    auto it = gDownloadMap.find(id);
    if (it == gDownloadMap.end()) return "";
    return std::string(it->second->filename);
}

std::string InternetSystem::GetDownloadURLID(std::string id)
{
    std::lock_guard<std::mutex> lock(gDownloadMapMutex);
    auto it = gDownloadMap.find(id);
    if (it == gDownloadMap.end()) return "";
    return std::string(it->second->url);
}

int InternetSystem::GetActiveDownloadCount()
{
    std::lock_guard<std::mutex> lock(gDownloadMapMutex);
    int count = 0;
    for (auto& pair : gDownloadMap)
    {
        if (pair.second->pending)
            count++;
    }
    return count;
}

void InternetSystem::CancelDownloadID(std::string id)
{
    std::lock_guard<std::mutex> lock(gDownloadMapMutex);
    auto it = gDownloadMap.find(id);
    if (it != gDownloadMap.end())
    {
        delete it->second;
        gDownloadMap.erase(it);
    }
}
