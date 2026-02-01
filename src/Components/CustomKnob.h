#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class CustomKnob : public juce::Slider
{
public:
    CustomKnob(const juce::String& labelText);
    ~CustomKnob() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setValueSuffix(const juce::String& suffix) { valueSuffix = suffix; }

private:
    juce::String label;
    juce::String valueSuffix;
    juce::Image knobImage;

    void drawLabel(juce::Graphics& g, juce::Rectangle<float> area);
    void drawValueBox(juce::Graphics& g, juce::Rectangle<float> area);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomKnob)
};
