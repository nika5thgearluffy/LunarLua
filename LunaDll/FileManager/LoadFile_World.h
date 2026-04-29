#ifndef LoadFileWorld_hhh
#define LoadFileWorld_hhh

#include <vector>
#include <string>

struct WorldMusicBoxes_Struct
{
    std::string customMusicFile;

    WorldMusicBoxes_Struct()
    {
        Reset();
    }

    void Reset()
    {
        customMusicFile = "";
    }
};

extern WorldMusicBoxes_Struct WorldMusicBoxes[999];

#endif
