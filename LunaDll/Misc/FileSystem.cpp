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
    // Specify the filesystem path and copy_options variables
    std::experimental::filesystem::path filePath1fs = filePath1;
    std::experimental::filesystem::path filePath2fs = filePath2;
    const auto optionsToUse = std::experimental::filesystem::copy_options::update_existing;

    // Now copy the file to the specified place!
    std::experimental::filesystem::copy_file(filePath1fs, filePath2fs, optionsToUse);
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
