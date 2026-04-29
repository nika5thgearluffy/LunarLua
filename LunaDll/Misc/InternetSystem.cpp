#include "InternetSystem.h"

#include <windows.h>
#include <wininet.h>
#include <future>
#include <fstream>

#include "../Globals.h"

// This downloads a file and parses it as a string
// [CLAUDE AI WAS USED FOR THIS PART OF THE CODE]
std::string Internet::DownloadURL(const std::string& url)
{
    std::string result;

    HINTERNET hInternet = InternetOpenA("LunarLua", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return result;

    HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hConnect)
    {
        InternetCloseHandle(hInternet);
        return result;
    }

    // Get total file size from headers
    DWORD contentLength = 0;
    DWORD bufLen = sizeof(DWORD);
    HttpQueryInfo(hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
        &contentLength, &bufLen, NULL);

    char buffer[4096];
    DWORD bytesRead = 0;
    DWORD totalRead = 0;

    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
    {
        result.append(buffer, bytesRead);
        totalRead += bytesRead;

        // Update global progress (0-100)
        if (contentLength > 0)
            gDownloadProgress = (int)((totalRead * 100) / contentLength);
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return result;
}

// Gets a filename from the specificed URL.
// [CLAUDE AI WAS USED FOR THIS PART OF THE CODE]
std::string Internet::GetFilenameFromURL(const std::string& url)
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

int Internet::DownloadProgress()
{
    return gDownloadProgress;
}

bool Internet::IsDownloading()
{
    return gDownloadPending;
}

std::string Internet::DownloadFilename()
{
    return gDownloadFilename;
}

// [CLAUDE AI WAS USED FOR THIS PART OF THE CODE]
void Internet::StartDownload(const std::string& url, const std::string& savePath)
{
    if (gDownloadPending)
        return;

    gDownloadPending = true;
    gDownloadFilename = GetFilenameFromURL(url);
    gDownloadProgress = 0;

    gDownloadFuture = std::async(std::launch::async, [url, savePath]() {
        std::string result = Internet::DownloadURL(url);

        if (!savePath.empty() && !result.empty())
        {
            std::ofstream file(savePath);
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
        gLunaLua.callEvent(downloadStart, url, savePath);
    }
    
}

// Call this every frame from your game tick
// [CLAUDE AI WAS USED FOR THIS PART OF THE CODE]
void Internet::Poll()
{
    if (!gDownloadPending)
        return;

    // Check if download is done without blocking
    if (gDownloadFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        std::string result = gDownloadFuture.get();
        gDownloadPending = false;

        if (gLunaLua.isValid())
        {
            std::shared_ptr<Event> downloadComplete = std::make_shared<Event>("onDownloadComplete", false);
            downloadComplete->setDirectEventName("onDownloadComplete");
            downloadComplete->setLoopable(false);
            gLunaLua.callEvent(downloadComplete, result);
        }

        gDownloadFilename = "";
        gDownloadProgress = 0;
    }
}