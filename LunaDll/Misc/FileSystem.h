#ifndef FileSystem_hhh
#define FileSystem_hhh

#include <windows.h>
#include <string>

namespace FileSystem{
    double GetFileSize(std::string file);
    void CopyFile(std::string filePath1, std::string filePath2);

    std::string OpenDialogAndGetFilepath();

    bool CreateDirectory(std::string pathToDirectory);
    bool DirectoryExists(std::string directory);
    bool RemoveDirectory(std::string path);

    std::string GetMD5Hash(std::string filePath);

    bool HasEnoughDiskSpace(std::wstring path, int requiredBytesOfUse);
    
    bool CreateAFile(std::string path, std::string fileData = "");
    
    std::string Base64Encode(std::string data);
    std::string Base64Decode(std::string data);
};

#endif