#include "MonitorSystem.h"

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

#include "../Rendering/WindowSizeHandler.h"

// 10 monitors is the max Windows supports
MonitorSystemMonitors monitorInformation[9];
int numberOfMonitors = -1;

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    numberOfMonitors = numberOfMonitors + 1;
    MONITORINFOEX info;
    info.cbSize = sizeof(info);

    if (GetMonitorInfo(hMonitor, &info))
    {
        monitorInformation[numberOfMonitors].index = numberOfMonitors + 1;
        monitorInformation[numberOfMonitors].monitorLeft = info.rcMonitor.left;
        monitorInformation[numberOfMonitors].monitorRight = info.rcMonitor.right;
        monitorInformation[numberOfMonitors].monitorTop = info.rcMonitor.top;
        monitorInformation[numberOfMonitors].monitorBottom = info.rcMonitor.bottom;

        monitorInformation[numberOfMonitors].monitorX = (info.rcMonitor.left - info.rcMonitor.right);
        monitorInformation[numberOfMonitors].monitorY = (info.rcMonitor.top - info.rcMonitor.bottom);
        monitorInformation[numberOfMonitors].monitorWidth = (info.rcMonitor.right - info.rcMonitor.left);
        monitorInformation[numberOfMonitors].monitorHeight = (info.rcMonitor.bottom - info.rcMonitor.top);

        monitorInformation[numberOfMonitors].monitorName = (WCHAR)info.szDevice;
    }

    return TRUE;
}

void MonitorSystem::SetupMonitors()
{
    for(int i = 0; i <= 9; i++)
    {
        monitorInformation[i].Reset();
    }

    bool monitorSuccess = EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);

    int monitorCountToCompare = GetSystemMetrics(SM_CMONITORS);
    numberOfMonitors = numberOfMonitors + 1; // To make sure that Lua is compatible with this
    if(numberOfMonitors != monitorCountToCompare)
    {
        MessageBoxA(NULL, "Looks like the size comparison is false", "Test", NULL);
    }
}

int MonitorSystem::GetScreenXPosition()
{
    RECT rect;
    GetWindowRect(gMainWindowHwnd, &rect);
    int x = rect.left;
    return x;
}

int MonitorSystem::GetScreenYPosition()
{
    RECT rect;
    GetWindowRect(gMainWindowHwnd, &rect);
    int y = rect.top;
    return y;
}

int MonitorSystem::GetScreenCenterXPosition(int monitorID)
{
    // Get window size
    auto windowSize = gWindowSizeHandler.getWindowSize();

    int dupeCheckLeft = monitorInformation[monitorID - 1].monitorLeft - monitorInformation[monitorID - 1].monitorLeft;
    int posX = monitorInformation[monitorID - 1].monitorWidth / 2 - (windowSize.x - dupeCheckLeft) / 2;

    return monitorInformation[monitorID - 1].monitorLeft + posX;
}

int MonitorSystem::GetScreenCenterXPosition()
{
    // Without a monitor arg, use the default monitor (1)
    return MonitorSystem::GetScreenCenterXPosition(1);
}

int MonitorSystem::GetScreenCenterYPosition(int monitorID)
{
    // Get window size
    auto windowSize = gWindowSizeHandler.getWindowSize();

    int dupeCheckTop = monitorInformation[monitorID - 1].monitorTop - monitorInformation[monitorID - 1].monitorTop;
    int posY = monitorInformation[monitorID - 1].monitorHeight / 2 - (windowSize.y - dupeCheckTop) / 2;

    return monitorInformation[monitorID - 1].monitorTop + posY;
}

int MonitorSystem::GetScreenCenterYPosition()
{
    // Without a monitor arg, use the default monitor (1)
    return MonitorSystem::GetScreenCenterYPosition(1);
}

int MonitorSystem::GetScreenResolutionWidth(int monitorID)
{
    return monitorInformation[monitorID - 1].monitorWidth;
}

int MonitorSystem::GetScreenResolutionWidth()
{
    // Without a monitor arg, use the default monitor (1)
    return monitorInformation[0].monitorWidth;
}

int MonitorSystem::GetScreenResolutionHeight(int monitorID)
{
    return monitorInformation[monitorID - 1].monitorHeight;
}

int MonitorSystem::GetScreenResolutionHeight()
{
    // Without a monitor arg, use the default monitor (1)
    return monitorInformation[0].monitorHeight;
}

int MonitorSystem::GetScreenX(int monitorID)
{
    return monitorInformation[monitorID - 1].monitorX;
}

int MonitorSystem::GetScreenX()
{
    // Without a monitor arg, use the default monitor (1)
    return monitorInformation[0].monitorX;
}

int MonitorSystem::GetScreenY(int monitorID)
{
    return monitorInformation[monitorID - 1].monitorY;
}

int MonitorSystem::GetScreenY()
{
    // Without a monitor arg, use the default monitor (1)
    return monitorInformation[0].monitorY;
}

// This will center the window to the screen. Useful for auto-moving the window to the center if you want to reset where X2 was when starting up the engine. monitorID will center to that specific monitor.
void MonitorSystem::CenterWindow(int monitorID)
{
    int x, y;
    x = MonitorSystem::GetScreenCenterXPosition(monitorID);
    y = MonitorSystem::GetScreenCenterYPosition(monitorID);

    // When getting everything set, center the window!
    SetWindowPos(gMainWindowHwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void MonitorSystem::CenterWindow()
{
    // Without a monitor, use the default for centering
    MonitorSystem::CenterWindow(1);
}

// Raw function for SetWindowPos
static void SetWindowPos(int x, int y, int width, int height)
{
    SetWindowPos(gMainWindowHwnd, NULL, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOREDRAW);
}

// Sets the window position of the game.
void MonitorSystem::SetWindowPosition(int x, int y)
{
    SetWindowPos(x, y, MonitorSystem::getWindowWidth(), MonitorSystem::getWindowHeight());
}

// Sets the window size of the game.
void MonitorSystem::setWindowSize(int width, int height)
{
    SetWindowPos(MonitorSystem::GetScreenXPosition(), MonitorSystem::GetScreenYPosition(), width, height);
    gWindowSizeHandler.SetWindowSize(width, height);
    gWindowSizeHandler.Recalculate();
}

// Sets the window scale of the game.
void MonitorSystem::setWindowScale(int scale)
{
    gWindowSizeHandler.SetNewWindowScale(scale);
    gWindowSizeHandler.Recalculate();
}

// Get the window width of the game.
int MonitorSystem::getWindowWidth()
{
    RECT rect;
    if(GetWindowRect(gMainWindowHwnd, &rect))
    {
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        return width;
    }
    return 0;
}

// Get the window height of the game.
int MonitorSystem::getWindowHeight()
{
    RECT rect;
    if(GetWindowRect(gMainWindowHwnd, &rect))
    {
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        return height;
    }
    return 0;
}


static luabind::object getMonitorInfo(int monitorID, lua_State *L)
{
    int realIndex = monitorID - 1;
    if(monitorID < 1 && monitorID > numberOfMonitors || std::to_string(monitorInformation[realIndex].index).length() == 0)
    {
        luabind::object outData = luabind::newtable(L);
        outData["invalid"] = "monitor";
        return outData;
    }
    else
    {
        wchar_t monitorNameDraft = monitorInformation[realIndex].monitorName;
        std::wstring monitorName(1, monitorNameDraft);
        luabind::object outData = luabind::newtable(L);

        outData["index"] = monitorInformation[realIndex].index;
        outData["monitorLeft"] = monitorInformation[realIndex].monitorLeft;
        outData["monitorRight"] = monitorInformation[realIndex].monitorRight;
        outData["monitorTop"] = monitorInformation[realIndex].monitorTop;
        outData["monitorBottom"] = monitorInformation[realIndex].monitorBottom;
        outData["monitorX"] = monitorInformation[realIndex].monitorX;
        outData["monitorY"] = monitorInformation[realIndex].monitorY;
        outData["monitorWidth"] = monitorInformation[realIndex].monitorWidth;
        outData["monitorHeight"] = monitorInformation[realIndex].monitorHeight;
        outData["monitorName"] = WStr2Str(monitorName);

        return outData;
    }
}

luabind::object MonitorSystem::GetMonitorInfo(int monitorID, lua_State *L)
{
    return getMonitorInfo(monitorID, L);
}
