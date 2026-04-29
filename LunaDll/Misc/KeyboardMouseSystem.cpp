#include "KeyboardMouseSystem.h"

#include <windows.h>
#include "../Globals.h"
#include "../GlobalFuncs.h"
#include "../Defines.h"

#include <lua.hpp>
#include <luabind/luabind.hpp>
#include <luabind/function.hpp>
#include <luabind/class.hpp>
#include <luabind/detail/call_function.hpp>
#include "../LuaMain/LuaHelper.h"

// -------------
// **KEYBOARDS**
// -------------

UINT numberOfKeyboards = 0;
keyboardDevices keyboardDeviceList[9];
keyboardDevices keyboardDevice;

static luabind::object getKeyboardDeviceThings(int index, lua_State *L)
{
    int realIndex = index - 1;
    if(index < 1 && index > numberOfKeyboards || std::to_string(keyboardDeviceList[index].index).length() == 0)
    {
        luabind::object outData = luabind::newtable(L);
        outData["invalid"] = "keyboard";
        return outData;
    }
    else
    {
        luabind::object outData = luabind::newtable(L);
        outData["index"] = keyboardDeviceList[realIndex].index;
        outData["deviceName"] = keyboardDeviceList[realIndex].deviceName;
        outData["keyboardMode"] = keyboardDeviceList[realIndex].keyboardMode;
        outData["functionKeys"] = keyboardDeviceList[realIndex].functionKeys;
        outData["indicators"] = keyboardDeviceList[realIndex].indicators;
        outData["totalKeys"] = keyboardDeviceList[realIndex].totalKeys;
        outData["keyboardType"] = keyboardDeviceList[realIndex].keyboardType;
        outData["keyboardSubtype"] = keyboardDeviceList[realIndex].keyboardSubtype;
        outData["keyboardID"] = keyboardDeviceList[realIndex].keyboardID;

        return outData;
    }
}

luabind::object KeyboardSystem::GetInfoFromIdx(int index, lua_State *L)
{
    return getKeyboardDeviceThings(index, L);
}

void KeyboardSystem::GetAllRawKeyboards()
{
    for(int i = 0; i <= 9; i++)
    {
        keyboardDeviceList[i].Reset();
    }

    std::string error = "";

    GetRawInputDeviceList(NULL, &numberOfKeyboards, sizeof(RAWINPUTDEVICELIST));
    if(numberOfKeyboards <= 0)
    {
        numberOfKeyboards = 0;
        error = "If you get this error, that means that a keyboard cannot be found for the engine. Please connect a keyboard to play SMBX2.";
        LunaMsgBox::ShowA(NULL, error.c_str(), "Keyboard Error", NULL);
    }

    PRAWINPUTDEVICELIST pRawInputDeviceList = NULL;
    pRawInputDeviceList = new RAWINPUTDEVICELIST[sizeof(RAWINPUTDEVICELIST) * numberOfKeyboards];

    if(pRawInputDeviceList == NULL)
    {
        error = "If you get this error, that means that the memory cannot be allocated to generate the keyboard list for the engine. If you see this, contact Nika.";
        LunaMsgBox::ShowA(NULL, error.c_str(), "Keyboard Error", NULL);
    }

    int nResult = 0;
    nResult = GetRawInputDeviceList(pRawInputDeviceList, &numberOfKeyboards, sizeof(RAWINPUTDEVICELIST));
    int keyboardCount = -1;

    for (UINT i = 0; i < numberOfKeyboards; i++)
    {
        // Get character count for device name
        UINT nBufferSize = 0;
        nResult = GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice, RIDI_DEVICENAME, NULL, &nBufferSize);

        if(nResult < 0)
        {
            continue;
        }

        // Allocate memory for device name
        WCHAR* wcDeviceName = new WCHAR[nBufferSize + 1];

        if (wcDeviceName == NULL)
        {
            continue;
        }

        // Get name
        nResult = GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice, RIDI_DEVICENAME, wcDeviceName, &nBufferSize);

        // Did we get the entire buffer?
        if (nResult < 0)
        {
            continue;
        }

        // Set device info & buffer size
        RID_DEVICE_INFO rdiDeviceInfo;
        rdiDeviceInfo.cbSize = sizeof(RID_DEVICE_INFO);
        nBufferSize = rdiDeviceInfo.cbSize;

        // Get device info
        nResult = GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice, RIDI_DEVICEINFO, &rdiDeviceInfo, &nBufferSize);
        
        // Did we get the entire buffer?
        if (nResult < 0)
        {
            continue;
        }

        std::wstring deviceName = (std::wstring)wcDeviceName;
        std::string deviceNameFinal = WStr2Str(deviceName);

        if (rdiDeviceInfo.dwType == RIM_TYPEKEYBOARD && deviceNameFinal.length() > 0 && rdiDeviceInfo.keyboard.dwNumberOfKeysTotal > 0 && (int)pRawInputDeviceList[i].hDevice > 0)
        {
            keyboardCount = keyboardCount + 1;
            keyboardDeviceList[keyboardCount].index = keyboardCount;
            keyboardDeviceList[keyboardCount].deviceName = deviceNameFinal.c_str();
            keyboardDeviceList[keyboardCount].keyboardMode = rdiDeviceInfo.keyboard.dwKeyboardMode;
            keyboardDeviceList[keyboardCount].functionKeys = rdiDeviceInfo.keyboard.dwNumberOfFunctionKeys;
            keyboardDeviceList[keyboardCount].indicators = rdiDeviceInfo.keyboard.dwNumberOfIndicators;
            keyboardDeviceList[keyboardCount].totalKeys = rdiDeviceInfo.keyboard.dwNumberOfKeysTotal;
            keyboardDeviceList[keyboardCount].keyboardType = rdiDeviceInfo.keyboard.dwType;
            keyboardDeviceList[keyboardCount].keyboardSubtype = rdiDeviceInfo.keyboard.dwSubType;
            keyboardDeviceList[keyboardCount].keyboardID = (int)pRawInputDeviceList[i].hDevice;
        }
        else
        {
            continue;
        }
    }

    numberOfKeyboards = keyboardCount;
    
    // If we have more than 10 keyboards inserted, let the player know
    if(numberOfKeyboards > 9)
    {
        numberOfKeyboards = 9;
        error = "Unfortunately, only 10 keyboards can be connected at maximum for the engine. Please disconnect an extra keyboard, then refresh the device status again.";
        LunaMsgBox::ShowA(NULL, error.c_str(), "Keyboard Error", NULL);
    }

    // Critical errors, in case
    if(numberOfKeyboards == (UINT)-1)
    {
        
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            error = "An error has occured while getting keyboard information. The error code is " + std::to_string(GetLastError()) + ". Please check out https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes for a complete list of error codes to see what went wrong.";
            LunaMsgBox::ShowA(NULL, error.c_str(), "Keyboard Error", NULL);
        }
        free(pRawInputDeviceList);
    }
}

int KeyboardSystem::GetCount()
{
    return (int)numberOfKeyboards;
}

int KeyboardSystem::GetKeyboardIDListing(int id)
{
    for(int i = 1; i <= KeyboardSystem::GetCount(); i++)
    {
        if(keyboardDeviceList[i - 1].keyboardID == id)
        {
            return i;
        }
    }
    return -1;
}

int KeyboardSystem::GetKeyboardToPressKeysWith(HANDLE hDevice)
{
    int finalKey = -1;
    int hDeviceInt = (int)hDevice;
    for(int i = 1; i <= KeyboardSystem::GetCount(); i++)
    {
        if(keyboardDeviceList[i - 1].keyboardID == hDeviceInt)
        {
            finalKey = keyboardDeviceList[i - 1].keyboardID;
        }
    }
    return finalKey;
}

// ----------
// **MOUSES**
// ----------

UINT numberOfMouses = 0;
mouseDevices mouseDeviceList[9];

static luabind::object getMouseDeviceThings(int index, lua_State *L)
{
    int realIndex = index - 1;
    if(index < 1 && index > numberOfMouses || std::to_string(mouseDeviceList[index].index).length() == 0)
    {
        luabind::object outData = luabind::newtable(L);
        outData["invalid"] = "mouse";
        return outData;
    }
    else
    {
        luabind::object outData = luabind::newtable(L);
        outData["index"] = mouseDeviceList[realIndex].index;
        outData["deviceName"] = mouseDeviceList[realIndex].deviceName;
        outData["mouseBitfield"] = mouseDeviceList[realIndex].mouseBitfield;
        outData["buttonCount"] = mouseDeviceList[realIndex].buttonCount;
        outData["sampleRate"] = mouseDeviceList[realIndex].sampleRate;
        outData["hasAHorizontalWheel"] = mouseDeviceList[realIndex].hasAHorizontalWheel;
        outData["mouseID"] = mouseDeviceList[realIndex].mouseID;

        return outData;
    }
}

luabind::object MouseSystem::GetInfoFromIdx(int index, lua_State *L)
{
    return getMouseDeviceThings(index, L);
}

void MouseSystem::GetAllRawMouses()
{
    for(int i = 0; i <= 9; i++)
    {
        mouseDeviceList[i].Reset();
    }

    std::string error = "";

    GetRawInputDeviceList(NULL, &numberOfMouses, sizeof(RAWINPUTDEVICELIST));

    PRAWINPUTDEVICELIST pRawInputDeviceList = NULL;
    pRawInputDeviceList = new RAWINPUTDEVICELIST[sizeof(RAWINPUTDEVICELIST) * numberOfMouses];

    if(pRawInputDeviceList == NULL)
    {
        error = "If you get this error, that means that the memory cannot be allocated to generate the mouse list for the engine. If you see this, contact Nika.";
        LunaMsgBox::ShowA(NULL, error.c_str(), "Mouse Error", NULL);
    }

    int nResult = 0;
    nResult = GetRawInputDeviceList(pRawInputDeviceList, &numberOfMouses, sizeof(RAWINPUTDEVICELIST));
    int mouseCount = -1;

    for (UINT i = 0; i < numberOfMouses; i++)
    {
        // Get character count for device name
        UINT nBufferSize = 0;
        nResult = GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice, RIDI_DEVICENAME, NULL, &nBufferSize);

        if(nResult < 0)
        {
            continue;
        }

        // Allocate memory for device name
        WCHAR* wcDeviceName = new WCHAR[nBufferSize + 1];

        if (wcDeviceName == NULL)
        {
            continue;
        }

        // Get name
        nResult = GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice, RIDI_DEVICENAME, wcDeviceName, &nBufferSize);

        // Did we get the entire buffer?
        if (nResult < 0)
        {
            continue;
        }

        // Set device info & buffer size
        RID_DEVICE_INFO rdiDeviceInfo;
        rdiDeviceInfo.cbSize = sizeof(RID_DEVICE_INFO);
        nBufferSize = rdiDeviceInfo.cbSize;

        // Get device info
        nResult = GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice, RIDI_DEVICEINFO, &rdiDeviceInfo, &nBufferSize);

        // Did we get the entire buffer?
        if (nResult < 0)
        {
            continue;
        }

        std::wstring deviceName = (std::wstring)wcDeviceName;
        std::string deviceNameFinal = WStr2Str(deviceName);

        if (rdiDeviceInfo.dwType == RIM_TYPEMOUSE && deviceNameFinal.length() > 0 && rdiDeviceInfo.mouse.dwNumberOfButtons > 0 && (int)pRawInputDeviceList[i].hDevice > 0)
        {
            mouseCount = mouseCount + 1;
            mouseDeviceList[mouseCount].index = mouseCount;
            mouseDeviceList[mouseCount].deviceName = deviceNameFinal.c_str();
            mouseDeviceList[mouseCount].mouseBitfield = rdiDeviceInfo.mouse.dwId;
            mouseDeviceList[mouseCount].buttonCount = rdiDeviceInfo.mouse.dwNumberOfButtons;
            mouseDeviceList[mouseCount].sampleRate = rdiDeviceInfo.mouse.dwSampleRate;
            mouseDeviceList[mouseCount].hasAHorizontalWheel = rdiDeviceInfo.mouse.fHasHorizontalWheel;
            mouseDeviceList[mouseCount].mouseID = (int)pRawInputDeviceList[i].hDevice;
        }
        else
        {
            continue;
        }
    }

    numberOfMouses = mouseCount;
    
    // If we have more than 10 keyboards inserted, let the player know
    if(numberOfMouses > 9)
    {
        numberOfMouses = 9;
        error = "Unfortunately, only 10 mice can be connected at maximum for the engine. Please disconnect an extra mouse, then refresh the device status again.";
        LunaMsgBox::ShowA(NULL, error.c_str(), "Mouse Error", NULL);
    }

    // Critical errors, in case
    if(numberOfMouses == (UINT)-1)
    {
        
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            error = "An error has occured while getting mouse information. The error code is " + std::to_string(GetLastError()) + ". Please check out https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes for a complete list of error codes to see what went wrong.";
            LunaMsgBox::ShowA(NULL, error.c_str(), "Mouse Error", NULL);
        }
        free(pRawInputDeviceList);
    }
}

int MouseSystem::GetCount()
{
    return (int)numberOfMouses;
}

// -----------
// **DEVICES**
// -----------

bool KeyboardMouseSystem::RegisterDevices()
{
    // [CLAUDE AI WAS USED IN THIS PART OF THE CODE]
    KeyboardMouseSystem::RefreshDevices();

    UINT realKeyCount = KeyboardSystem::GetCount();  // not static
    RAWINPUTDEVICE* rid = new RAWINPUTDEVICE[realKeyCount];

    for(UINT i = 0; i < realKeyCount; i++)
    {
        rid[i].usUsagePage = 0x01;
        rid[i].usUsage = 0x06;
        rid[i].dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
        rid[i].hwndTarget = gMainWindowHwnd;
    }

    // cbSize is the size of a single RAWINPUTDEVICE, not the total
    int success = RegisterRawInputDevices(rid, realKeyCount, sizeof(RAWINPUTDEVICE));

    delete[] rid;
    return success;
}

void KeyboardMouseSystem::RefreshDevices()
{
    KeyboardSystem::GetAllRawKeyboards();
    MouseSystem::GetAllRawMouses();
}

//----