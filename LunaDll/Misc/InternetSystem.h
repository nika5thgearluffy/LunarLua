#ifndef InternetSystem_hhh
#define InternetSystem_hhh

#include <windows.h>
#include <string>

namespace Internet{
    std::string DownloadURL(const std::string& url);

    std::string GetFilenameFromURL(const std::string& url);

    int DownloadProgress();
    bool IsDownloading();
    std::string DownloadFilename();

    void StartDownload(const std::string& url, const std::string& savePath = "");
    void Poll();
};

#endif
