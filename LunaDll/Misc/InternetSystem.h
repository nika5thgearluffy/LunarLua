#ifndef InternetSystem_hhh
#define InternetSystem_hhh

#include <windows.h>
#include <string>

namespace InternetSystem{
    std::string DownloadURL(std::string url);

    std::string GetFilenameFromURL(std::string url);

    int DownloadProgress();
    bool IsDownloading();
    std::string DownloadFilename();
    std::string DownloadURL();

    void StartDownload(std::string url, std::string savePath);
    void Poll();
    
    // New multi-download functions
    std::string StartDownloadID(std::string url, std::string savePath = "");
    bool IsDownloadingID(std::string id);
    int GetDownloadProgressID(std::string id);
    std::string GetDownloadFilenameID(std::string id);
    std::string GetDownloadURLID(std::string id);
    void CancelDownloadID(std::string id);
    int GetActiveDownloadCount();
};

#endif
