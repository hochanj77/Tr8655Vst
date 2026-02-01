#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class VUMeter : public juce::Component, public juce::Timer
{
public:
    VUMeter(const juce::String& labelText = "");
    ~VUMeter() override = default;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void setLevel(float newLevel);

private:
    juce::String label;
    juce::Image frameImage;
    float currentLevel = 0.0f;
    float displayLevel = 0.0f;
    float peakLevel = 0.0f;
    int peakHoldCounter = 0;

    static constexpr int numSegments = 24;
    static constexpr int peakHoldFrames = 60;

    juce::Colour getSegmentOnColour(int index) const;
    juce::Colour getSegmentOffColour(int index) const;
    juce::Colour getSegmentGlowColour(int index) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VUMeter)
};
