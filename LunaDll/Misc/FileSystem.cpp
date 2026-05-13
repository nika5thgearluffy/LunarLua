#include "FileSystem.h"

#include <windows.h>
#include <shobjidl.h>
#include <wincrypt.h>

#include <string>
#include <io.h>
#include <iostream>
#include <fstream>

#include "../GlobalFuncs.h"
#include "../LuaMain/LunaPathValidator.h"

#pragma comment(lib, "advapi32.lib")

// Retrieves the size of a file
double FileSystem::GetFileSize(std::string file)
{
    std::wstring path = Str2WStr(file);
    std::wifstream theFile(path, std::ios::binary| std::ios::ate);
    return theFile.tellg();
}

// Copies a file. Copying to a destination gets handled later under lockdown.lua
void FileSystem::CopyFile(std::string filePath1, std::string filePath2)
{
    // Let's copy with an extended MAX_PATH
    std::string filePath1Extend = "\\\\?\\" + filePath1;
    std::string filePath2Extend = "\\\\?\\" + filePath2;

    // Now copy the file to the specified place!
    CopyFileExA(filePath1Extend.c_str(), filePath2Extend.c_str(), NULL, NULL, NULL, COPY_FILE_NO_BUFFERING);
}

std::string FileSystem::OpenDialogAndGetFilepath()
{
    // Create the file path variable
    PWSTR pszFilePath;

    // Initalize Co
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    // Create the file dialog variable
    IFileOpenDialog *pFileOpen;
    
    // Create the FileOpenDialog object.
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

    // Check if we succeeded
    if (SUCCEEDED(hr))
    {
        // Show the Open dialog box.
        hr = pFileOpen->Show(NULL);

        // Get the file name from the dialog box.
        if (SUCCEEDED(hr))
        {
            // Get the file open result
            IShellItem *pItem;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr))
            {
                // Now get the filepath
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                // If succeeded, uninitalize everything...
                if (SUCCEEDED(hr))
                {
                    CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }
        pFileOpen->Release();
    }
    CoUninitialize();

    // ...then return the filepath
    return WStr2Str(pszFilePath);
}

// Creates a directory. Folder creation gets handled later under lockdown.lua
bool FileSystem::CreateDirectory(std::string pathToDirectory)
{
    // Extend the MAX_PATH
    std::string pathToDirectoryExtend = "\\\\?\\" + pathToDirectory;

    // Create the directory
    CreateDirectoryA(pathToDirectoryExtend.c_str(), NULL);

    // Check for an error
    int hasError = GetLastError();
    if (hasError == ERROR_ALREADY_EXISTS || hasError == ERROR_PATH_NOT_FOUND)
    {
        return false;
    }
    return true;
}

// Checks if a directory exists or not.
bool FileSystem::DirectoryExists(std::string directory)
{
    DWORD dwAttrib = GetFileAttributesA(directory.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// Gets an MD5 hash of a file.
std::string FileSystem::GetMD5Hash(std::string filePath)
{
    // Open file
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
        return "";

    // Read file contents
    std::string contents((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());

    // Set up crypto provider
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        return "";

    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
    {
        CryptReleaseContext(hProv, 0);
        return "";
    }

    // Hash the data
    if (!CryptHashData(hHash, (BYTE*)contents.data(), contents.size(), 0))
    {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return "";
    }

    // Get hash bytes
    BYTE hashBytes[16];
    DWORD hashLen = 16;
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hashBytes, &hashLen, 0))
    {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return "";
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    // Convert to hex string
    char hexStr[33] = { 0 };
    for (int i = 0; i < 16; i++)
        sprintf_s(&hexStr[i * 2], 3, "%02x", hashBytes[i]);

    return std::string(hexStr);
}

// Returns if the bytes of a file has enough disk space to create a file.
bool FileSystem::HasEnoughDiskSpace(std::wstring path, int requiredBytesOfUse)
{
    uint64_t requiredBytes = requiredBytesOfUse;
    ULARGE_INTEGER freeBytes;
    ULARGE_INTEGER totalBytes;
    ULARGE_INTEGER totalFreeBytes;
    
    if (GetDiskFreeSpaceExW(path.c_str(), &freeBytes, &totalBytes, &totalFreeBytes))
    {
        return freeBytes.QuadPart >= requiredBytes;
    }
    return false;
}

// Creates a file, with optionally data to include.
bool FileSystem::CreateAFile(std::string path, std::string fileData)
{
    std::ofstream file(path);
    if (fileData.empty())
    {
        return file.is_open();
    }
    else
    {
        const int chunkSize = 4096;
        size_t offset = 0;
        if (!file.is_open())
        {
            return false;
        }
        while (offset < fileData.size())
        {
            size_t toWrite = std::min((size_t)chunkSize, fileData.size() - offset);
            file.write(fileData.c_str() + offset, toWrite);
            offset += toWrite;
        }
        file.close();
        return true;
    }
    return false;
}
