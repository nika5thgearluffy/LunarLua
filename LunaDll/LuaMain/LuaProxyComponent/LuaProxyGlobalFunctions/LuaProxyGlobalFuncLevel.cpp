#include "../../LuaProxy.h"
#include "../../../Defines.h"
#include "../../../GlobalFuncs.h"

#include "../../../Episode/EpisodeMain.h"

#include "../../../SMBXInternal/Types.h"
#include "../../../SMBXInternal/Variables.h"
#include "../../../SMBXInternal/Functions.h"

std::string LuaProxy::Level::filename()
{
    return (std::string)GM_LVLFILENAME_PTR;
}

std::string LuaProxy::Level::name()
{
    return (std::string)GM_LVLNAME_PTR;
}

// This loads a level from the world map
void LuaProxy::Level::worldLoad(std::string fileName, int warpIdx)
{
    gEpisodeMain.LoadWorldMapLevel(fileName, warpIdx);
}

void LuaProxy::Level::worldLoad(std::string fileName)
{
    gEpisodeMain.LoadWorldMapLevel(fileName, 0);
}

// Forms paths when specifying a level ID.
void LuaProxy::Level::worldPath(int levelID, bool shouldSkipAnimation)
{
    // Don't forget to set level beat code for applying where to unlock paths!
    using namespace SMBX13;
    Functions::LevelPath(levelID, 5, (VB6Bool)shouldSkipAnimation);
}
