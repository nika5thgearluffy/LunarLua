//*** Globals2.h - It has some globals in it ***
#ifndef globals2_hhhhhh
#define globals2_hhhhhh

#include <string>
#include <atomic>
#include <cstdint>
#include <mutex>

struct EpisodeSettings
{
    // These should be used for getting the path of the episode for the episode.ini system
    std::wstring episodeDirectory;
    std::wstring episodeDirectoryWithoutRoot;

    // Splash screen customization!
    bool usingCustomSplash;
    std::wstring episodeBootImage;

    // Boot sound customization!
    std::wstring episodeBootSoundCustom;
    int episodeBootSoundID;

    // Use the 1.3 credits?
    bool displayOriginalCredits;

    // Overrides the cheat and no save "feature"
    bool canCheatAndSave;

    // Episode resolution settings
    int episodeWidth;
    int episodeHeight;

    // Whether we should use an overlay when unfocused and paused
    bool showPauseOverlay;

    // Default life count when starting the episode for the first time/getting a game over
    int defaultLifeCount;
    // Should we decrease lives after death?
    bool decreaseLivesAfterDeath;

    // Whether the player should have an easier powerdown instead of one based off SMB1. This phases out anotherPowerdownLibrary.
    bool easierPowerdown;

    // If the episode should suppress close-to-the-limit message boxes, this should be set to true.
    bool suppressCloseLimitMessages;

    EpisodeSettings() :
        episodeDirectory(L""), episodeDirectoryWithoutRoot(L""),
        usingCustomSplash(false), episodeBootImage(L""),
        episodeBootSoundCustom(L""), episodeBootSoundID(29),
        displayOriginalCredits(false),
        canCheatAndSave(false),
        episodeWidth(800), episodeHeight(600),
        showPauseOverlay(true),
        defaultLifeCount(3), decreaseLivesAfterDeath(true),
        easierPowerdown(false),
        suppressCloseLimitMessages(false)
    {
    }
};

extern EpisodeSettings gEpisodeSettings;

#endif
