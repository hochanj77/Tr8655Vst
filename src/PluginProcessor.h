#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "SamplerEngine/SamplerEngine.h"
#include "Presets/PresetSamples.h"

class TR8655AudioProcessor : public juce::AudioProcessor
{
public:
    TR8655AudioProcessor();
    ~TR8655AudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Public API
    void loadPreset(int presetIndex);
    void loadCustomSample(const juce::File& wavFile);
    int getCurrentPresetIndex() const { return currentPresetIndex; }
    juce::String getCurrentSampleName() const { return samplerEngine.getCurrentSampleName(); }

    // Thread-safe meter values
    std::atomic<float> inputLevel  { 0.0f };
    std::atomic<float> outputLevel { 0.0f };
    std::atomic<float> bassLevel   { 0.0f };
    std::atomic<int>   lastMidiNote{ -1 };
    std::atomic<bool>  midiActivity{ false };
    std::atomic<bool>  engaged     { true };

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void applyEffectsChain(juce::AudioBuffer<float>& buffer);

    SamplerEngine samplerEngine;
    int currentPresetIndex = 0;
    juce::String customSamplePath;

    // DSP
    juce::dsp::ProcessSpec currentSpec{};
    juce::dsp::IIR::Filter<float> subFilterL, subFilterR;
    float envelopeFollower = 0.0f;
    float prevSample = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TR8655AudioProcessor)
};
