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

// Base64 encode/decode in C++
std::string FileSystem::Base64Encode(std::string data)
{
    static const char table[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string result;
    int i = 0;
    unsigned char buf[3];
    
    for (size_t pos = 0; pos < data.size(); pos++)
    {
        buf[i++] = data[pos];
        if (i == 3)
        {
            result += table[(buf[0] & 0xfc) >> 2];
            result += table[((buf[0] & 0x03) << 4) | ((buf[1] & 0xf0) >> 4)];
            result += table[((buf[1] & 0x0f) << 2) | ((buf[2] & 0xc0) >> 6)];
            result += table[buf[2] & 0x3f];
            i = 0;
        }
    }
    
    if (i > 0)
    {
        memset(buf + i, 0, 3 - i);
        result += table[(buf[0] & 0xfc) >> 2];
        result += table[((buf[0] & 0x03) << 4) | ((buf[1] & 0xf0) >> 4)];
        result += (i > 1) ? table[((buf[1] & 0x0f) << 2) | ((buf[2] & 0xc0) >> 6)] : '=';
        result += '=';
    }
    
    return result;
}

std::string FileSystem::Base64Decode(std::string data)
{
    static const int table[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1
    };
    
    std::string result;
    int val = 0, valb = -8;
    
    for (unsigned char c : data)
    {
        if (table[c] == -1) break;
        val = (val << 6) + table[c];
        valb += 6;
        if (valb >= 0)
        {
            result += (char)((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    
    return result;
}
