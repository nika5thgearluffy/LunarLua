#include "../LuaProxy.h"
#include "../../GlobalFuncs.h"
#include "../../Misc/AsmPatch.h"

LuaProxy::Console::Console() :
    m_isDefaultConsole(true)
{}

void LuaProxy::Console::print(const std::string& outText)
{
    DebugPrint("%s", outText.c_str());
}

void LuaProxy::Console::println(const std::string& outText)
{
    DebugPrint("%s\n", outText.c_str());
}

void LuaProxy::Console::clear()
{
    HANDLE hStdout;
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DebugClear(hStdout);
}

void LuaProxy::Console::show()
{
    if (GetConsoleWindow() == NULL)
    {
        InitDebugConsole();
        AsmRange::StartChecking();
    }
}

bool LuaProxy::Console::isEnabled()
{
    if (GetConsoleWindow() != NULL)
    {
        return true;
    }
    return false;
}
