#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class Visualizer : public juce::Component, public juce::Timer
{
public:
    Visualizer();
    ~Visualizer() override = default;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void setIntensity(float v) { targetIntensity = juce::jlimit(0.0f, 1.0f, v); }
    void setBassIntensity(float v) { targetBassIntensity = juce::jlimit(0.0f, 1.0f, v); }
    void setDropHighlight(bool h) { dropHighlight = h; repaint(); }

private:
    void drawWaves(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawWaveLayer(juce::Graphics& g, juce::Rectangle<float> bounds,
                       juce::Colour color, float speed, float freq,
                       float amplitudeMultiplier, float warpIntensity,
                       float opacity, bool isFilled, float indexOffset);

    juce::Image sphereImage;
    float time = 0.0f;
    float intensity = 0.3f;
    float bassIntensity = 0.3f;
    float targetIntensity = 0.3f;
    float targetBassIntensity = 0.3f;
    bool dropHighlight = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Visualizer)
};
