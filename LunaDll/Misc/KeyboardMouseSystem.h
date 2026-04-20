#ifndef KeyboardMouseSystem_hhh
#define KeyboardMouseSystem_hhh

#include <windows.h>

#include <lua.hpp>
#include <luabind/luabind.hpp>
#include <luabind/function.hpp>
#include <luabind/class.hpp>
#include <luabind/detail/call_function.hpp>
#include "../LuaMain/LuaHelper.h"

/*************************************************************************/
/* Keyboard/Mouse/Device Main Functions                                  */
/*************************************************************************/
extern void HID_GetAllRawKeyboards();
extern int HID_GetKeyboardCount();
extern luabind::object HID_GetKeyboardInfoFromIdx(int index, lua_State *L);

extern UINT numberOfKeyboards;
struct keyboardDevices
{
    const char* deviceName;
    int keyboardMode;
    int functionKeys;
    int indicators;
    int totalKeys;
    int keyboardType;
    int keyboardSubtype;
    int index;
    int keyboardID;

    // Constructor
    keyboardDevices()
    {
        Reset();
    }

    // Reset function
    void Reset()
    {
        deviceName = "";
        keyboardMode = 0;
        functionKeys = 0;
        indicators = 0;
        totalKeys = 0;
        keyboardType = 0;
        keyboardSubtype = 0;
        index = 0;
        keyboardID = 0;
    }
};
extern keyboardDevices keyboardDeviceList[9];
extern keyboardDevices keyboardDevice;
extern int GetKeyboardIDListing(int id);
extern int GetKeyboardToPressKeysWith(HANDLE hDevice);

/***********************/

extern UINT numberOfMouses;
struct mouseDevices
{
    int index;
    const char* deviceName;
    int mouseBitfield;
    int buttonCount;
    int sampleRate;
    bool hasAHorizontalWheel;
    int mouseID;

    // Constructor
    mouseDevices()
    {
        Reset();
    }

    // Reset function
    void Reset()
    {
        index = 0;
        deviceName = "";
        mouseBitfield = 0;
        buttonCount = 0;
        sampleRate = 0;
        hasAHorizontalWheel = false;
        mouseID = 0;
    }
};
extern mouseDevices mouseDeviceList[9];
luabind::object HID_GetMouseInfoFromIdx(int index, lua_State *L);

extern void HID_GetAllRawMouses();
extern int HID_GetMouseCount();

/***********************/

extern bool HID_RegisterDevices();
extern void HID_RefreshDevices();

/***********************/

#endif
