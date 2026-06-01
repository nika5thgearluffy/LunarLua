#ifndef InternetSystem_hhh
#define InternetSystem_hhh

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_  // prevent winsock.h from being included by windows.h
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include <windows.h>

#include <string>

namespace InternetSystem{
    std::string DownloadURL(std::string url);
    std::string GetFilenameFromURL(std::string url);

    void Poll();
    
    // Multi-download functions
    std::string StartDownloadID(std::string url, std::string savePath = "");
    bool IsDownloadingID(std::string id);
    int GetDownloadProgressID(std::string id);
    std::string GetDownloadFilenameID(std::string id);
    std::string GetDownloadURLID(std::string id);
    void CancelDownloadID(std::string id);
    int GetActiveDownloadCount();

    // UDP connections
    std::string GetLocalIP();
    bool SendPacket(std::string targetIP, int port, std::string data);
    bool StartListening(int port);
    std::string ReceivePacket();
    void CloseSockets();
    bool EnableBroadcast();
    bool SendBroadcast(int port, std::string data);
    std::string ReceiveBroadcast(std::string senderIP);
    std::string ReceiveBroadcastData();
    std::string GetLastBroadcastSender();
};

#endif
