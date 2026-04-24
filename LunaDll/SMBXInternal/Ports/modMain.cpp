#include "../Types.h"
#include "../Variables.h"
#include "../Functions.h"
#include "../Ports.h"
#include "../../Misc/VB6RNG.h"
#include "../../Misc/VB6Logic.h"

// Macro timers that can be adjusted to whatever value the episode creator desires
int SMBX13::Ports::MacroTimer_SMB3ExitEnd = 100;

// Functions to get/set timer values.
int SMBX13::Ports::MacroTimer_SMB3ExitEnd_Get()
{
    using namespace SMBX13::Ports;
    return MacroTimer_SMB3ExitEnd;
}

void SMBX13::Ports::MacroTimer_SMB3ExitEnd_Set(int val)
{
    using namespace SMBX13::Ports;
    MacroTimer_SMB3ExitEnd = val;
}

// This is an automatically translated copy of UpdateMacro() from modMain.bas
// This accounts for:
// - Changing timers of ending macros to whatever you want
void __stdcall SMBX13::Ports::UpdateMacro() {
    using namespace SMBX13::Types;
    using namespace SMBX13::Vars;
    using namespace SMBX13::Functions;
    #pragma warning( push )
    #pragma warning( disable: 4244 ) // Disable loss of precision warning
    int16_t A = 0;
    VB6Bool OnScreen = false;
    // SMB3 Exit
    if (LevelMacro == 1) {
        for (A = 1; A <= numPlayers; A++) {
                auto& _ = Player[A];
                if ((_.Location.X < level[_.Section].Width) && (_.Dead == false)) {
                    OnScreen = true;
                    {
                        auto& a = _.Controls;
                        a.Down = false;
                        a.Drop = false;
                        a.Jump = false;
                        a.Left = false;
                        a.Right = true;
                        a.Run = false;
                        a.Up = false;
                        a.Start = false;
                        a.AltJump = false;
                        a.AltRun = false;
                    }
                    if ((_.Wet > 0) && (_.CanJump == true)) {
                        if (_.Location.SpeedY > 1) { _.Controls.Jump = true; }
                    }
                }
                else {
                    _.Location.SpeedY = -Physics.PlayerGravity;
                    {
                        auto& a = _.Controls;
                        a.Down = false;
                        a.Drop = false;
                        a.Jump = false;
                        a.Left = false;
                        a.Right = true;
                        a.Run = false;
                        a.Up = false;
                        a.Start = false;
                        a.AltJump = false;
                        a.AltRun = false;
                    }
                }
        }
        if (OnScreen == false) {
            LevelMacroCounter = (LevelMacroCounter + 1);
            if (LevelMacroCounter >= 100) {
                LevelBeatCode = 1;
                LevelMacro = 0;
                LevelMacroCounter = 0;
                EndLevel = true;
            }
        }
    }
    // SMB3 Orb
    else if (LevelMacro == 2) {
        for (A = 1; A <= numPlayers; A++) {
            {
                auto& _ = Player[A].Controls;
                _.Down = false;
                _.Drop = false;
                _.Jump = false;
                _.Left = false;
                _.Right = false;
                _.Run = false;
                _.Up = false;
                _.Start = false;
                _.AltJump = false;
                _.AltRun = false;
            }
        }
        LevelMacroCounter = (LevelMacroCounter + 1);
        if (LevelMacroCounter >= 460) {
            LevelBeatCode = 2;
            EndLevel = true;
            LevelMacro = 0;
            LevelMacroCounter = 0;
        }
    }
    // SMW Keyhole
    else if (LevelMacro == 3) {
        float tempTime = 0.0f;
        float gameTime = 0.0f;
        do
        {
            tempTime = (Timer - ::floor(Timer));
            if ((tempTime > (gameTime + 0.01)) || (tempTime < gameTime)) {
                gameTime = tempTime;
                DoEvents();
                UpdateGraphics();
                UpdateSound();
                BlockFrames();
                LevelMacroCounter = (LevelMacroCounter + 1);
            }
        }
        while (LevelMacroCounter < 300);
        LevelBeatCode = 4;
        EndLevel = true;
        LevelMacro = 0;
        LevelMacroCounter = 0;
    }
    // SMB2 Orb
    else if (LevelMacro == 4) {
        for (A = 1; A <= numPlayers; A++) {
            {
                auto& _ = Player[A].Controls;
                _.Down = false;
                _.Drop = false;
                _.Jump = false;
                _.Left = false;
                _.Right = false;
                _.Run = false;
                _.Up = false;
                _.Start = false;
                _.AltJump = false;
                _.AltRun = false;
            }
        }
        LevelMacroCounter = (LevelMacroCounter + 1);
        if (LevelMacroCounter >= 300) {
            LevelBeatCode = 5;
            EndLevel = true;
            LevelMacro = 0;
            LevelMacroCounter = 0;
        }
    }
    // Game End
    else if (LevelMacro == 5) {
        for (A = 1; A <= numPlayers; A++) {
            {
                auto& _ = Player[A].Controls;
                _.Down = false;
                _.Drop = false;
                _.Jump = false;
                _.Left = false;
                _.Right = false;
                _.Run = false;
                _.Up = false;
                _.Start = false;
                _.AltJump = false;
                _.AltRun = false;
            }
        }
        LevelMacroCounter = (LevelMacroCounter + 1);
        if (LevelMacroCounter == 250) {
            PlaySound(45);
        }
        if (LevelMacroCounter >= 800) {
            EndLevel = true;
            LevelMacro = 0;
            LevelMacroCounter = 0;
            if (TestLevel == false) {
                GameOutro = true;
                BeatTheGame = true;
                SaveGame();
                MenuMode = 0;
                MenuCursor = 0;
            }
        }
    }
    // Star Exit
    else if (LevelMacro == 6) {
        for (A = 1; A <= numPlayers; A++) {
            {
                auto& _ = Player[A].Controls;
                _.Down = false;
                _.Drop = false;
                _.Jump = false;
                _.Left = false;
                _.Right = false;
                _.Run = false;
                _.Up = false;
                _.Start = false;
                _.AltJump = false;
                _.AltRun = false;
            }
        }
        LevelMacroCounter = (LevelMacroCounter + 1);
        if (LevelMacroCounter >= 300) {
            LevelBeatCode = 7;
            LevelMacro = 0;
            LevelMacroCounter = 0;
            EndLevel = true;
        }
    }
    // SMW Exit
    else if (LevelMacro == 7) {
        for (A = 1; A <= numPlayers; A++) {
            {
                auto& _ = Player[A];
                if ((_.Location.X < level[_.Section].Width) && (_.Dead == false)) {
                    {
                        auto& a = _.Controls;
                        a.Down = false;
                        a.Drop = false;
                        a.Jump = false;
                        a.Left = false;
                        a.Right = true;
                        a.Run = false;
                        a.Up = false;
                        a.Start = false;
                        a.AltJump = false;
                        a.AltRun = false;
                    }
                }
                else {
                    _.Location.SpeedY = -Physics.PlayerGravity;
                    {
                        auto& a = _.Controls;
                        a.Down = false;
                        a.Drop = false;
                        a.Jump = false;
                        a.Left = false;
                        a.Right = true;
                        a.Run = false;
                        a.Up = false;
                        a.Start = false;
                        a.AltJump = false;
                        a.AltRun = false;
                    }
                }
            }
        }
        LevelMacroCounter = (LevelMacroCounter + 1);
        if (LevelMacroCounter >= 630) {
            LevelBeatCode = 8;
            LevelMacro = 0;
            LevelMacroCounter = 0;
            EndLevel = true;
        }
    }
    #pragma warning( pop )
}