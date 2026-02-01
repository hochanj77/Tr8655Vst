#pragma once
#include "PluginProcessor.h"
#include "Components/CustomKnob.h"
#include "Components/VUMeter.h"
#include "Components/Visualizer.h"
#include "Components/PresetSelector.h"

class EngageButton : public juce::Component
{
public:
    EngageButton();
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent&) override;
    bool isActive() const { return active; }
    void setActive(bool a) { active = a; repaint(); }
    std::function<void(bool)> onToggle;
private:
    bool active = true;
    juce::Image onImage, offImage;
};

class TR8655AudioProcessorEditor : public juce::AudioProcessorEditor,
                                    public juce::FileDragAndDropTarget,
                                    public juce::DragAndDropContainer,
                                    public juce::Timer
{
public:
    explicit TR8655AudioProcessorEditor(TR8655AudioProcessor&);
    ~TR8655AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;

private:
    TR8655AudioProcessor& processorRef;

    CustomKnob gainKnob   {"Gain"};
    CustomKnob driveKnob  {"Drive"};
    CustomKnob threshKnob {"Thresh"};
    CustomKnob ratioKnob  {"Ratio"};
    CustomKnob subKnob    {"Sub"};
    CustomKnob punchKnob  {"Punch"};
    CustomKnob mixKnob    {"Mix"};
    CustomKnob outputKnob {"Output"};

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> gainAtt, driveAtt, threshAtt, ratioAtt;
    std::unique_ptr<Attachment> subAtt, punchAtt, mixAtt, outputAtt;

    VUMeter inMeter{"IN"}, subMeter{"SUB"};
    Visualizer visualizer;
    PresetSelector presetSelector;
    EngageButton engageButton;

    juce::Image logoImage;
    bool isFileDragOver = false;

    void drawBackground(juce::Graphics& g);
    void drawHeader(juce::Graphics& g);
    void drawSectionHeaders(juce::Graphics& g);
    void drawFooter(juce::Graphics& g);
    void drawScrew(juce::Graphics& g, float x, float y);

    void drawIconSignal(juce::Graphics& g, float x, float y, float size);
    void drawIconDynamics(juce::Graphics& g, float x, float y, float size);
    void drawIconEqualizer(juce::Graphics& g, float x, float y, float size);
    void drawIconMaster(juce::Graphics& g, float x, float y, float size);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TR8655AudioProcessorEditor)
};
