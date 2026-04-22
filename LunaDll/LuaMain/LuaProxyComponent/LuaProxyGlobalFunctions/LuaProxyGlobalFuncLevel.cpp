#include "../../LuaProxy.h"
#include "../../../Defines.h"
#include "../../../GlobalFuncs.h"

#include "../../../Episode/EpisodeMain.h"

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
