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
};

#endif
