#include "InternetSystem.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <windows.h>

#include <wininet.h>

#include <future>
#include <fstream>

#include "../Globals.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ws2_32.lib")

// Socket packets
// [CLAUDE AI WAS USED FOR THIS PART OF THE CODE]
static SOCKET gSendSocket = INVALID_SOCKET;
static SOCKET gRecvSocket = INVALID_SOCKET;
static bool gWsaInitialized = false;

// [CLAUDE AI WAS USED FOR THIS PART OF THE CODE]
static void EnsureWSAInit()
{
    if (!gWsaInitialized)
    {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        gWsaInitialized = true;
    }
}

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

// Call this every frame from your game tick
// [CLAUDE AI WAS USED FOR THIS PART OF THE CODE]
void InternetSystem::Poll()
{
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

    if (gLunaLua.isValid())
    {
        std::shared_ptr<Event> downloadStart = std::make_shared<Event>("onDownloadStart", false);
        downloadStart->setDirectEventName("onDownloadStart");
        downloadStart->setLoopable(false);
        gLunaLua.callEvent(downloadStart, url, InternetSystem::GetFilenameFromURL(url));
    }

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


// Gets the current local IP address.
std::string InternetSystem::GetLocalIP()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    addrinfo hints = {};
    hints.ai_family = AF_INET;  // IPv4
    addrinfo* result = nullptr;
    getaddrinfo(hostname, nullptr, &hints, &result);

    std::string ip = "";
    if (result)
    {
        sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(result->ai_addr);
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr->sin_addr, ipStr, sizeof(ipStr));
        ip = ipStr;
        freeaddrinfo(result);
    }

    return ip;
}

bool InternetSystem::SendPacket(std::string targetIP, int port, std::string data)
{
    EnsureWSAInit();

    if (gSendSocket == INVALID_SOCKET)
        gSendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, targetIP.c_str(), &addr.sin_addr);

    int result = sendto(gSendSocket, data.c_str(), data.size(), 0,
        reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

    return result != SOCKET_ERROR;
}

bool InternetSystem::StartListening(int port)
{
    EnsureWSAInit();

    if (gRecvSocket != INVALID_SOCKET)
        closesocket(gRecvSocket);

    gRecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (gRecvSocket == INVALID_SOCKET)
        return false;

    // Set non-blocking so it doesn't hang the game loop
    u_long mode = 1;
    ioctlsocket(gRecvSocket, FIONBIO, &mode);

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    return bind(gRecvSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != SOCKET_ERROR;
}

std::string InternetSystem::ReceivePacket()
{
    if (gRecvSocket == INVALID_SOCKET)
        return "";

    char buffer[4096];
    sockaddr_in senderAddr = {};
    int senderLen = sizeof(senderAddr);

    int received = recvfrom(gRecvSocket, buffer, sizeof(buffer) - 1, 0,
        reinterpret_cast<sockaddr*>(&senderAddr), &senderLen);

    if (received == SOCKET_ERROR)
        return "";  // no data available (non-blocking)

    buffer[received] = '\0';
    return std::string(buffer, received);
}

void InternetSystem::CloseSockets()
{
    if (gSendSocket != INVALID_SOCKET)
    {
        closesocket(gSendSocket);
        gSendSocket = INVALID_SOCKET;
    }
    if (gRecvSocket != INVALID_SOCKET)
    {
        closesocket(gRecvSocket);
        gRecvSocket = INVALID_SOCKET;
    }
    if (gWsaInitialized)
    {
        WSACleanup();
        gWsaInitialized = false;
    }
}

bool InternetSystem::EnableBroadcast()
{
    EnsureWSAInit();

    if (gSendSocket == INVALID_SOCKET)
        gSendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Enable broadcast on the socket
    BOOL broadcastEnable = TRUE;
    return setsockopt(gSendSocket, SOL_SOCKET, SO_BROADCAST,
        reinterpret_cast<char*>(&broadcastEnable), sizeof(broadcastEnable)) != SOCKET_ERROR;
}

bool InternetSystem::SendBroadcast(int port, std::string data)
{
    EnsureWSAInit();

    if (gSendSocket == INVALID_SOCKET)
        gSendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_BROADCAST;  // 255.255.255.255

    int result = sendto(gSendSocket, data.c_str(), data.size(), 0,
        reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

    return result != SOCKET_ERROR;
}

std::string InternetSystem::ReceiveBroadcast(std::string senderIP)
{
    if (gRecvSocket == INVALID_SOCKET)
        return "";

    char buffer[4096];
    sockaddr_in senderAddr = {};
    int senderLen = sizeof(senderAddr);

    int received = recvfrom(gRecvSocket, buffer, sizeof(buffer) - 1, 0,
        reinterpret_cast<sockaddr*>(&senderAddr), &senderLen);

    if (received == SOCKET_ERROR)
        return "";

    buffer[received] = '\0';

    // Get sender IP so you know who sent the broadcast
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr));
    senderIP = std::string(ipStr);

    return std::string(buffer, received);
}

static std::string gLastBroadcastSenderIP = "";

std::string InternetSystem::ReceiveBroadcastData()
{
    return InternetSystem::ReceiveBroadcast(gLastBroadcastSenderIP);
}

std::string InternetSystem::GetLastBroadcastSender()
{
    return gLastBroadcastSenderIP;
}


// Reads a Git commit hash from the specified folder under "data"
std::string InternetSystem::GetGitCommitHash(std::string folderToUse)
{
    std::string gitDir = folderToUse + "\\.git\\";
    std::string headPath = gitDir + "HEAD";
    
    // Check if .git folder exists
    if (!file_existsX(headPath))
        return "unknown";
    
    // Read HEAD file
    std::string headContent;
    if (!readFile(headContent, headPath))
        return "unknown";
    
    // Trim whitespace/newlines
    headContent.erase(headContent.find_last_not_of(" \t\r\n") + 1);
    
    // Check if HEAD contains a ref or a direct hash
    // ref format: "ref: refs/heads/branchname"
    // direct hash format: "a1b2c3d4e5f6..."
    if (headContent.substr(0, 5) == "ref: ")
    {
        // Follow the ref to get the actual hash
        std::string refPath = gitDir + headContent.substr(5);
        // Convert forward slashes to backslashes
        std::replace(refPath.begin(), refPath.end(), '/', '\\');
        
        std::string hashContent;
        if (!readFile(hashContent, refPath))
        {
            // Ref file not found, try packed-refs
            std::string packedRefsPath = gitDir + "packed-refs";
            std::string packedRefs;
            if (readFile(packedRefs, packedRefsPath))
            {
                std::string refName = headContent.substr(5);
                std::istringstream stream(packedRefs);
                std::string line;
                while (std::getline(stream, line))
                {
                    if (line.find(refName) != std::string::npos)
                    {
                        // Line format: "hash refname"
                        size_t spacePos = line.find(' ');
                        if (spacePos != std::string::npos)
                            return line.substr(0, spacePos);
                    }
                }
            }
            return "unknown";
        }
        
        // Trim and return hash
        hashContent.erase(hashContent.find_last_not_of(" \t\r\n") + 1);
        return hashContent;
    }
    else
    {
        // HEAD directly contains the hash (detached HEAD state)
        return headContent;
    }
}

// Short version (first 7 chars, like GitHub shows)
std::string InternetSystem::GetGitCommitHashShort(std::string folderToUse)
{
    std::string hash = InternetSystem::GetGitCommitHash(folderToUse);
    if (hash == "unknown" || hash.length() < 7)
        return hash;
    return hash.substr(0, 7);
}

// Get current branch name too while we're at it
std::string InternetSystem::GetGitBranch(std::string folderToUse)
{
    std::string headPath = folderToUse + "\\.git\\HEAD";
    
    if (!file_existsX(headPath))
        return "unknown";
    
    std::string headContent;
    if (!readFile(headContent, headPath))
        return "unknown";
    
    headContent.erase(headContent.find_last_not_of(" \t\r\n") + 1);
    
    if (headContent.substr(0, 5) == "ref: ")
    {
        // Extract branch name from "ref: refs/heads/branchname"
        std::string ref = headContent.substr(5);
        size_t lastSlash = ref.find_last_of('/');
        if (lastSlash != std::string::npos)
            return ref.substr(lastSlash + 1);
        return ref;
    }
    
    return "detached";
}
