#include <windows.h>
#include <time.h>
#include <cctype>
#include <cstdarg>
#include <mutex>
#include <future>
#include <atomic>
#include "Globals.h"
#include "LevelCodes/LevelCodes.h"
#include "Shlwapi.h"
#include "UserSaves/UserSaving.h"
#include "GlobalFuncs.h"
#include "Globals2.h"

HINSTANCE gHInstance;

HWND gMainWindowHwnd = NULL;
bool gMainWindowFocused = false;
bool gMainWindowUnfocusPending = false;
bool gMainWindowUnfocusOverlay = false;

// Global settings
bool gLunaEnabled;
bool gPrintErrorsToScreen;
bool gIsOverworld;
bool gHook_SkipTestMsgBox;
bool gAutostartRan = false;
SMBXHUDSettings gSMBXHUDSettings = {
    false,    // skip
    WHUD_ALL, // overworldHudControlFlag
    false     // skipStarCount
};
bool gRenderBGOFlag = true;
bool gRenderSizableFlag = true;
bool gRenderBackgroundFlag = true;

// Settings for glitch fixes
bool gDisablePlayerFilterBounceFix = false;
bool gDisableNPCRespawnBugFix = false;

// Other gameplay settings
bool gLavaIsWeak = false;

// Flag for returning from gameover screen
bool gDidGameOver = false;

// Global variables
int	gFrames;

bool gCamerasInitialised = false;

int gLastDownPress;
int gDownTapped;
int gLastUpPress;
int gUpTapped;
int gLastLeftPress;
int gLeftTapped;
int gLastRightPress;
int gRightTapped;

int gLastJumpPress;
int gJumpTapped;
int gLastRunPress;
int gRunTapped;

BYTE gKeyState[9][256] = { 0 };

HDC	ghMemDC;			// General use screen-compatible DC
HBITMAP	ghGeneralDIB;	// General use screen-sized DIB
DWORD* gpScreenBits;	// Pointer to screen bits for general use DIB

// Mouse cursor
HCURSOR  gCustomCursor = nullptr;
bool     gCustomCursorHide = false;

LvlEnum gLevelEnum;

// First two array entries are players 1+2
// Last two array entries are copies of the above for last tick
KeyMap gRawKeymap[4] = {0};

bool gEscPressed = false;
bool gEscPressedRegistered = false;

bool gIsTestModePauseActive;

// Global obj

AutocodeManager gAutoMan;

CSpriteManager gSpriteMan;

CellManager gCellMan;

SavedVariableBank gSavedVarBank;

MciEmulator gMciEmulator;

StartupSettings gStartupSettings;

HardcodedGraphicsManager gGraphManager;

FreeImageInit        gFreeImgInit;

GeneralLunaConfig    gGeneralConfig;

LunaGameControllerManager gLunaGameControllerManager;

CLunaLua gLunaLua;

FILE* conout = nullptr;


// Paths
std::string  gAppPathANSI;
std::string  gAppPathUTF8;
std::wstring gAppPathWCHAR;
std::string  gCwdPathANSI;
std::string  gCwdPathUTF8;
std::wstring gCwdPathWCHAR;

// Printf-like message boxes
void printBoxW(const wchar_t *fmt, ...)
{
    wchar_t buf[32768];
    va_list ap;
    va_start(ap, fmt);
    std::vswprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    dbgbox(buf);
}

void printBoxA(const char *fmt, ...)
{
    char buf[32768];
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    dbgboxA(buf);
}

// Editor entity
std::string gEditorPlacedItem = "";
std::mutex g_editorIPCMutex;

bool gEpisodeLoadedOnBoot = false;
Characters gPlayerStoredCharacters[] = {CHARACTER_MARIO,CHARACTER_MARIO,CHARACTER_MARIO,CHARACTER_MARIO };

// Are we testing a level?
std::mutex g_testingLevelMutex;
bool gIsTestingLevel = false;

// Music/SFX counts
int gSoundEffectCount = 0;

int gMusicCountSpecial = 0;
int gMusicCountOverworld = 0;
int gMusicCountLevel = 0;

// Have we first booted the engine?
bool gFirstBooted = true;

// If this is true, onPlayerDie won't be called
bool gWarpingToLevelFromMap = false;

// True if the episode is a wldx
bool gInWldx = false;

// The user files directory.
std::string gUserFilesPathUTF8;
std::wstring gUserFilesPathWCHAR;

// Needed for USB detection and exiting SMBX2
HDEVNOTIFY hDevNotify;
HDEVNOTIFY hMonitorNotify;

// Set this to true to use the new system of player inputs.
bool gDisablePlayerKeysLegacy = false;

// Set to true for pending a download.
std::future<std::string> gDownloadFuture;
std::atomic<bool> gDownloadPending(false);
std::atomic<int> gDownloadProgress(0);
char gDownloadFilename[2048] = { 0 };
char gDownloadURL[2048] = { 0 };
char gDownloadSavePath[2048] = { 0 };

// New globals for multiple-download support
std::unordered_map<std::string, DownloadEntry*> gDownloadMap;
std::mutex gDownloadMapMutex;

// Set the max int value for Lua-related calls
int gLuaMaxInt = 2147483647;

// Background worker thread, which is used to run a loop process outside of the game loop
std::thread* gBackgroundWorkerThread = nullptr;
std::atomic<bool> gBackgroundWorkerRunning(false);

// Legacy pause menu & message box variables.
bool gLegacyPauseMenu = true;
bool gLegacyMessageBox = true;

// Clipboard history stuff
bool gUseManualHistory = false;
