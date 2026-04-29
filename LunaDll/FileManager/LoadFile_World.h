#ifndef LoadFileWorld_hhh
#define LoadFileWorld_hhh

#include <vector>
#include <string>

struct WorldMusicBoxes_Entries
{
    std::string customMusicFile;
};

struct WorldMusicBoxes_List
{
    std::vector<WorldMusicBoxes_Entries> entries;
};

extern WorldMusicBoxes_List WorldMusicBoxes;

#endif
