#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

class PresetSelector : public juce::Component
{
public:
    PresetSelector();
    ~PresetSelector() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setPresetName(const juce::String& name);
    void setPresetList(const juce::StringArray& names);
    void setCurrentIndex(int index);
    int getCurrentIndex() const { return currentIndex; }

    std::function<void(int)> onPresetChanged;

private:
    juce::String currentPresetName = "DEEP SUB";
    juce::StringArray presetNames;
    int currentIndex = 0;
    juce::Image boxImage;

    juce::TextButton leftButton, rightButton;

    void prevPreset();
    void nextPreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetSelector)
};
