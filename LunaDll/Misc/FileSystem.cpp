#include "FileSystem.h"

#include <windows.h>
#include <shobjidl.h>

#include <string>
#include <io.h>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

#include "../GlobalFuncs.h"
#include "../LuaMain/LunaPathValidator.h"

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
