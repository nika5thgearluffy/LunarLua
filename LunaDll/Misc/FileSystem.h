#ifndef FileSystem_hhh
#define FileSystem_hhh

#include <windows.h>
#include <string>

namespace FileSystem{
    double GetFileSize(std::string file);
    void CopyFile(std::string filePath1, std::string filePath2);
    std::string OpenDialogAndGetFilepath();
};

#endif