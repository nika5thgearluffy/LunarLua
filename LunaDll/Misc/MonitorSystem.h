#ifndef MonitorSystem_hhh
#define MonitorSystem_hhh

#include <windows.h>

#include <lua.hpp>
#include <luabind/luabind.hpp>
#include <luabind/function.hpp>
#include <luabind/class.hpp>
#include <luabind/detail/call_function.hpp>
#include "../LuaMain/LuaHelper.h"

struct MonitorSystemMonitors
{
    int index;
    int monitorLeft;
    int monitorRight;
    int monitorTop;
    int monitorBottom;
    int monitorX;
    int monitorY;
    int monitorWidth;
    int monitorHeight;
    wchar_t monitorName[32];
    
    MonitorSystemMonitors()
    {
        Reset();
    }
    
    void Reset()
    {
        index = 0;
        monitorLeft = 0;
        monitorRight = 0;
        monitorTop = 0;
        monitorBottom = 0;
        monitorX = 0;
        monitorY = 0;
        monitorWidth = 0;
        monitorHeight = 0;
        monitorName[0] = L'\0';
    }
};

extern int numberOfMonitors;
extern MonitorSystemMonitors monitorInformation[9];

namespace MonitorSystem{
    void SetupMonitors();

    int GetCount();

    int GetScreenXPosition();
    int GetScreenYPosition();
    int GetScreenCenterXPosition(int monitorID);
    int GetScreenCenterXPosition();
    int GetScreenCenterYPosition(int monitorID);
    int GetScreenCenterYPosition();
    
    int GetScreenResolutionWidth(int monitorID);
    int GetScreenResolutionWidth();
    int GetScreenResolutionHeight(int monitorID);
    int GetScreenResolutionHeight();

    int GetScreenX(int monitorID);
    int GetScreenX();
    int GetScreenY(int monitorID);
    int GetScreenY();

    void CenterWindow(int monitorID);
    void CenterWindow();

    void SetWindowPosition(int x, int y);
    void setWindowSize(int width, int height);
    void setWindowScale(int scale);

    int getWindowWidth();
    int getWindowHeight();

    int FindWindowFromMonitor();

    int getWindowWidthFromResolution(int gameWidth, int gameHeight);
    int getWindowWidthFromResolution(int gameWidth);

    int getWindowHeightFromResolution(int gameWidth, int gameHeight);
    int getWindowHeightFromResolution(int gameHeight);

    float getDPIScale(int monitorID);
    float getDPIScale();

    int getRefreshRate(int monitorID);
    int getRefreshRate();

    std::string GetWindowTitle();
    
    luabind::object GetMonitorInfo(int monitorID, lua_State *L);
};

#endif
