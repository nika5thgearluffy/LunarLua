// [CLAUDE AI WAS USED FOR THIS ENTIRE CODE FILE]

#include "ClipboardSystem.h"

#include "../Globals.h"

#include <string>

static std::vector<std::string> gClipboardHistory;
static const int MAX_HISTORY = 25;  // same as Windows clipboard history limit

std::string ClipboardSystem::GetText()
{
    if (!OpenClipboard(gMainWindowHwnd))
        return "";

    std::string result;

    // Try Unicode first
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData != nullptr)
    {
        wchar_t* pText = static_cast<wchar_t*>(GlobalLock(hData));
        if (pText != nullptr)
        {
            result = WStr2Str(std::wstring(pText));
            GlobalUnlock(hData);
        }
    }
    else
    {
        // Fall back to ANSI
        hData = GetClipboardData(CF_TEXT);
        if (hData != nullptr)
        {
            char* pText = static_cast<char*>(GlobalLock(hData));
            if (pText != nullptr)
            {
                result = std::string(pText);
                GlobalUnlock(hData);
            }
        }
    }

    CloseClipboard();
    return result;
}

bool ClipboardSystem::SetText(std::string text)
{
    if (!OpenClipboard(gMainWindowHwnd))
        return false;

    EmptyClipboard();

    // Set Unicode text
    std::wstring wtext = Str2WStr(text);
    HGLOBAL hMemW = GlobalAlloc(GMEM_MOVEABLE, (wtext.size() + 1) * sizeof(wchar_t));
    if (hMemW != nullptr)
    {
        wchar_t* pMemW = static_cast<wchar_t*>(GlobalLock(hMemW));
        if (pMemW != nullptr)
        {
            memcpy(pMemW, wtext.c_str(), (wtext.size() + 1) * sizeof(wchar_t));
            GlobalUnlock(hMemW);
            SetClipboardData(CF_UNICODETEXT, hMemW);
        }
    }

    // Also set ANSI text for compatibility
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (hMem != nullptr)
    {
        char* pMem = static_cast<char*>(GlobalLock(hMem));
        if (pMem != nullptr)
        {
            memcpy(pMem, text.c_str(), text.size() + 1);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
        }
    }

    CloseClipboard();

    History_Add(text);
    return true;
}

bool ClipboardSystem::HasText()
{
    return IsClipboardFormatAvailable(CF_UNICODETEXT) != 0 ||
           IsClipboardFormatAvailable(CF_TEXT) != 0;
}

void ClipboardSystem::Clear()
{
    if (OpenClipboard(gMainWindowHwnd))
    {
        EmptyClipboard();
        CloseClipboard();
    }
}


void ClipboardSystem::History_Add(std::string text)
{
    if (text.empty()) return;
    
    // Don't add duplicates consecutively
    if (!gClipboardHistory.empty() && gClipboardHistory.front() == text)
        return;
    
    gClipboardHistory.insert(gClipboardHistory.begin(), text);
    
    // Keep history within limit
    if (gClipboardHistory.size() > MAX_HISTORY)
        gClipboardHistory.resize(MAX_HISTORY);
}

std::string ClipboardSystem::History_Get(int index)
{
    if (index < 0 || index >= (int)gClipboardHistory.size())
        return "";
    return gClipboardHistory[index];
}

int ClipboardSystem::History_Count()
{
    return (int)gClipboardHistory.size();
}

void ClipboardSystem::History_Clear()
{
    gClipboardHistory.clear();
}


void ClipboardSystem::Init()
{
    // Use the in-game clipboard history
    gUseManualHistory = true;

    // Start monitoring the history
    ClipboardSystem::StartMonitoring();
}

void ClipboardSystem::StartMonitoring()
{
    AddClipboardFormatListener(gMainWindowHwnd);
}

void ClipboardSystem::StopMonitoring()
{
    RemoveClipboardFormatListener(gMainWindowHwnd);
}
