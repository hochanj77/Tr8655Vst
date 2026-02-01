#pragma once
#include <juce_core/juce_core.h>
#include <vector>

struct PresetData
{
    juce::String name;
    juce::String category;
    juce::String sampleResourceName;
    int rootNote;
    float gain;
    float drive;
    float sub;
    float punch;
    float thresh;
    float ratio;
    float mix;
    float output;
};

inline std::vector<PresetData> getPresetList()
{
    return {
        // 808 Category
        { "DEEP SUB",  "808",  "DEEP_SUB_wav",  36, 45.0f, 12.0f, 80.0f, 82.0f, 18.0f, 4.0f,  100.0f, 0.0f },
        { "CLOUD",     "808",  "CLOUD_wav",     36, 40.0f, 20.0f, 75.0f, 70.0f, 20.0f, 4.0f,  100.0f, 0.0f },
        { "DIGI",      "808",  "DIGI_wav",      36, 50.0f, 35.0f, 60.0f, 65.0f, 22.0f, 5.0f,  100.0f, 0.0f },
        { "HOODTRAP",  "808",  "HOODTRAP_wav",  36, 48.0f, 45.0f, 85.0f, 90.0f, 24.0f, 6.0f,  100.0f, 0.0f },
        { "JERK",      "808",  "JERK_wav",      36, 42.0f, 30.0f, 70.0f, 75.0f, 18.0f, 4.0f,  100.0f, 0.0f },
        { "MINK",      "808",  "MINK_wav",      36, 38.0f, 15.0f, 78.0f, 68.0f, 16.0f, 3.0f,  100.0f, 0.0f },
        { "PLUGG",     "808",  "PLUGG_wav",     36, 55.0f, 55.0f, 65.0f, 85.0f, 28.0f, 5.0f,  100.0f, 0.0f },
        { "RAGE",      "808",  "RAGE_wav",      36, 60.0f, 70.0f, 90.0f, 95.0f, 30.0f, 8.0f,  100.0f, 0.0f },
        // Bass Category
        { "808 MAFIA", "Bass", "_08_MAFIA_wav", 36, 52.0f, 40.0f, 82.0f, 78.0f, 22.0f, 5.0f,  100.0f, 0.0f },
        { "REESE",     "Bass", "REESE_wav",     36, 45.0f, 25.0f, 75.0f, 60.0f, 20.0f, 4.0f,  100.0f, 0.0f },
        { "SYNTH",     "Bass", "SYNTH_wav",     36, 40.0f, 15.0f, 65.0f, 55.0f, 18.0f, 3.0f,  100.0f, 0.0f },
        { "ZAY",       "Bass", "ZAY_wav",       36, 48.0f, 35.0f, 80.0f, 72.0f, 24.0f, 5.0f,  100.0f, 0.0f },
    };
}

inline int getPresetCount() { return (int)getPresetList().size(); }
