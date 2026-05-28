#include <string>

#include "../Globals.h"
#include "../Episode/PlayerInput.h"

#define FFI_EXPORT(sig) __declspec(dllexport) sig __cdecl

extern "C" {
    // Sets the player keys state.
    FFI_EXPORT(bool) LunaLuaPlayerGetLegacyInputState()
    {
        return gDisablePlayerKeysLegacy;
    }

    FFI_EXPORT(void) LunaLuaPlayerSetLegacyInputState(bool toggle)
    {
        gDisablePlayerKeysLegacy = toggle;
    }



    // Button states for players
    FFI_EXPORT(bool) LunaLuaPlayerGetPlayerSpecial(int idx)
    {
        return g_playerInputPressing[idx - 1].specialPressing;
    }

    FFI_EXPORT(void) LunaLuaPlayerSetPlayerSpecial(int idx, bool toggle)
    {
        g_playerInputPressing[idx - 1].specialPressing = toggle;
    }

    FFI_EXPORT(bool) LunaLuaPlayerGetPlayerLTrigger(int idx)
    {
        return g_playerInputPressing[idx - 1].leftTriggerPressing;
    }

    FFI_EXPORT(void) LunaLuaPlayerSetPlayerLTrigger(int idx, bool toggle)
    {
        g_playerInputPressing[idx - 1].leftTriggerPressing = toggle;
    }

    FFI_EXPORT(bool) LunaLuaPlayerGetPlayerRTrigger(int idx)
    {
        return g_playerInputPressing[idx - 1].rightTriggerPressing;
    }

    FFI_EXPORT(void) LunaLuaPlayerSetPlayerRTrigger(int idx, bool toggle)
    {
        g_playerInputPressing[idx - 1].rightTriggerPressing = toggle;
    }
}