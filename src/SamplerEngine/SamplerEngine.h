#pragma once
#include <juce_audio_utils/juce_audio_utils.h>

class SamplerEngine
{
public:
    SamplerEngine();
    ~SamplerEngine() = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);

    void loadSample(const juce::File& wavFile);
    void loadSampleFromMemory(const void* data, size_t size, const juce::String& name);

    void setRootNote(int note) { rootNote = note; }
    int getRootNote() const { return rootNote; }

    juce::String getCurrentSampleName() const { return currentSampleName; }
    bool hasSampleLoaded() const { return sampleLoaded; }

    float getLastLevel() const { return lastLevel.load(); }

private:
    void addSoundFromReader(juce::AudioFormatReader* reader, const juce::String& name);
    void setupVoices();

    juce::Synthesiser sampler;
    juce::AudioFormatManager formatManager;
    int rootNote = 36;
    juce::String currentSampleName;
    bool sampleLoaded = false;
    double storedSampleRate = 44100.0;
    int storedSamplesPerBlock = 512;
    std::atomic<float> lastLevel { 0.0f };

    // Keep a copy of raw data for rebuilding
    juce::MemoryBlock currentSampleData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplerEngine)
};
