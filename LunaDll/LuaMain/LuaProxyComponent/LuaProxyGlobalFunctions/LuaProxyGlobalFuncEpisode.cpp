#include "../../LuaProxy.h"

#include "../../../Defines.h"
#include "../../../GlobalFuncs.h"
#include "../../../Globals.h"

#include "../../../Misc/VB6StrPtr.h"
#include "../../../Misc/VB6Bool.h"

#include "../../../SMBXInternal/Types.h"
#include "../../../SMBXInternal/Variables.h"
#include "../../../SMBXInternal/Functions.h"

#include "../../../Episode/EpisodeMain.h"

// luabind object that retrieves every episode from the world list, and creates a table from what was retrieved
static luabind::object getAllEpisodes(lua_State *L)
{
    luabind::object outData = luabind::newtable(L);
    {
        size_t counter = 0;

        for (int i = 0; i <= gEpisodeMain.GetEpisodeCount(); i++)
        {
            luabind::object e = luabind::newtable(L);
            // Episode name
            e["episodeName"] = WStr2Str(g_episodeList[i].episodeName);
            // Episode path
            e["episodePath"] = WStr2Str(g_episodeList[i].episodePath);
            // Episode world file
            e["episodeWorldFile"] = WStr2Str(g_episodeList[i].episodeWorldFile);
            outData[++counter] = e;
        }
    }
    
    return outData;
}

// returns a table of all the episodes in the SMBX2 world list.
luabind::object LuaProxy::Episode::list(lua_State *L)
{
    return getAllEpisodes(L);
}

// gets the currently running episode's ID, which can be used to retrieve the running episode's world info with Episode.list()[id]
int LuaProxy::Episode::id()
{
    return gEpisodeMain.GetEpisodeIdx();
}

int LuaProxy::Episode::count()
{
    return gEpisodeMain.GetEpisodeCount();
}

std::string LuaProxy::Episode::name()
{
    return WStr2Str(g_episodeList[gEpisodeMain.GetEpisodeIdx()].episodeName);
}

std::string LuaProxy::Episode::path()
{
    return WStr2Str(g_episodeList[gEpisodeMain.GetEpisodeIdx()].episodePath);
}

std::string LuaProxy::Episode::filename()
{
    return WStr2Str(g_episodeList[gEpisodeMain.GetEpisodeIdx()].episodeWorldFile);
}
