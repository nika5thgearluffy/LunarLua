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
#include "../Rendering/GL/GLContextManager.h"

// 10 monitors is the max Windows supports
MonitorSystemMonitors monitorInformation[9];
int numberOfMonitors = -1;

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
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

        // [CLAUDE AI WAS USED FOR THIS PART OF THE CODE]
        wcscpy_s(monitorInformation[numberOfMonitors].monitorName, sizeof(info.szDevice) / sizeof(WCHAR), info.szDevice);
    }

    numberOfMonitors = numberOfMonitors + 1;

    return TRUE;
}

void MonitorSystem::SetupMonitors()
{
    numberOfMonitors = 0; // Reset before enumeration

    for(int i = 0; i <= 9; i++)
    {
        monitorInformation[i].Reset();
    }

    bool monitorSuccess = EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
}

int MonitorSystem::GetCount()
{
    return GetSystemMetrics(SM_CMONITORS);
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

// [CLAUDE AI IS USED FOR THIS PART OF THE CODE]
int MonitorSystem::GetScreenCenterXPosition(int monitorID)
{
    float dpi = MonitorSystem::getDPIScale();
    auto currentSize = gWindowSizeHandler.getWindowSize();
    int windowWidth = MonitorSystem::getWindowWidthFromResolution(currentSize.x, currentSize.y) * dpi;

    int posX = monitorInformation[monitorID - 1].monitorWidth / 2 - (windowWidth) / 2;

    return monitorInformation[monitorID - 1].monitorLeft + posX;
}

int MonitorSystem::GetScreenCenterXPosition()
{
    // Without a monitor arg, use the default monitor (1)
    return MonitorSystem::GetScreenCenterXPosition(1);
}

// [CLAUDE AI IS USED FOR THIS PART OF THE CODE]
int MonitorSystem::GetScreenCenterYPosition(int monitorID)
{
    float dpi = MonitorSystem::getDPIScale();
    auto currentSize = gWindowSizeHandler.getWindowSize();
    int windowHeight = MonitorSystem::getWindowHeightFromResolution(currentSize.x, currentSize.y) * dpi;

    int posY = (monitorInformation[monitorID - 1].monitorHeight / 2) - (windowHeight / 2);

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
static void SetWindowPosMS(int x, int y, int width, int height)
{
    SetWindowPos(gMainWindowHwnd, NULL, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOREDRAW);
}

// Sets the window position of the game.
void MonitorSystem::SetWindowPosition(int x, int y)
{
    // Account for DPI and window width/height differences
    float dpi = MonitorSystem::getDPIScale();
    auto currentSize = gWindowSizeHandler.getWindowSize();
    int windowWidth = MonitorSystem::getWindowWidthFromResolution(currentSize.x, currentSize.y) * dpi;
    int windowHeight = MonitorSystem::getWindowHeightFromResolution(currentSize.x, currentSize.y) * dpi;

    // Now set the position
    SetWindowPosMS(x, y, windowWidth, windowHeight);
}

// Sets the window size of the game.
void MonitorSystem::setWindowSize(int width, int height)
{
    SetWindowPosMS(MonitorSystem::GetScreenXPosition(), MonitorSystem::GetScreenYPosition(), width, height);
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
    float dpi = MonitorSystem::getDPIScale();
    auto currentSize = gWindowSizeHandler.getWindowSize();
    int windowWidth = MonitorSystem::getWindowWidthFromResolution(currentSize.x, currentSize.y) * dpi;
    // Return the width
    return windowWidth;
}

// Get the window height of the game.
int MonitorSystem::getWindowHeight()
{
    // Account for DPI and window width/height differences
    float dpi = MonitorSystem::getDPIScale();
    auto currentSize = gWindowSizeHandler.getWindowSize();
    int windowHeight = MonitorSystem::getWindowHeightFromResolution(currentSize.x, currentSize.y) * dpi;
    // Return the height
    return windowHeight;
}

// Finds what monitor the window is on.
int MonitorSystem::FindWindowFromMonitor()
{
    for (int i = 0; i <= 9; i++)
    {
        if ((monitorInformation[i].monitorLeft <= MonitorSystem::GetScreenXPosition() && monitorInformation[i].monitorRight >= MonitorSystem::GetScreenXPosition()) && (monitorInformation[i].monitorTop <= MonitorSystem::GetScreenYPosition() && monitorInformation[i].monitorBottom >= MonitorSystem::GetScreenYPosition()))
        {
            return i + 1;
        }
    }
    return 0;
}

// Gets the window width based off the resolution itself.
// [CLAUDE AI IS USED FOR THIS PART OF THE CODE]
int MonitorSystem::getWindowWidthFromResolution(int gameWidth, int gameHeight)
{
    RECT rect = { 0, 0, gameWidth, gameHeight };
    DWORD style = GetWindowLong(gMainWindowHwnd, GWL_STYLE);
    DWORD exStyle = GetWindowLong(gMainWindowHwnd, GWL_EXSTYLE);
    BOOL hasMenu = (GetMenu(gMainWindowHwnd) != NULL);

    AdjustWindowRectEx(&rect, style, hasMenu, exStyle);
    return rect.right - rect.left;
}
int MonitorSystem::getWindowWidthFromResolution(int gameWidth)
{
    auto currentSize = gWindowSizeHandler.getWindowSize();
    return MonitorSystem::getWindowWidthFromResolution(gameWidth, g_GLContextManager.GetMainFBHeight());
}

// Gets the window height based off the resolution itself.
// [CLAUDE AI IS USED FOR THIS PART OF THE CODE]
int MonitorSystem::getWindowHeightFromResolution(int gameWidth, int gameHeight)
{
    RECT rect = { 0, 0, gameWidth, gameHeight };
    DWORD style = GetWindowLong(gMainWindowHwnd, GWL_STYLE);
    DWORD exStyle = GetWindowLong(gMainWindowHwnd, GWL_EXSTYLE);
    BOOL hasMenu = (GetMenu(gMainWindowHwnd) != NULL);

    AdjustWindowRectEx(&rect, style, hasMenu, exStyle);
    return rect.bottom - rect.top;
}
int MonitorSystem::getWindowHeightFromResolution(int gameHeight)
{
    return MonitorSystem::getWindowHeightFromResolution(g_GLContextManager.GetMainFBWidth(), gameHeight);
}

// Gets the DPI scale. This is accounted for 4K monitors and scaled resolutions.
// [CLAUDE AI IS USED FOR THIS PART OF THE CODE]
float MonitorSystem::getDPIScale()
{
    HDC hdc = GetDC(gMainWindowHwnd);
    float dpi = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
    ReleaseDC(gMainWindowHwnd, hdc);
    return dpi;
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
        std::wstring monitorName(monitorInformation[realIndex].monitorName);
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
