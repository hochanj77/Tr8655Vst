#include "SamplerEngine.h"

SamplerEngine::SamplerEngine()
{
    formatManager.registerBasicFormats();
    setupVoices();
}

void SamplerEngine::setupVoices()
{
    sampler.clearVoices();
    for (int i = 0; i < 8; ++i)
        sampler.addVoice(new juce::SamplerVoice());
}

void SamplerEngine::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    storedSampleRate = sampleRate;
    sampler.setCurrentPlaybackSampleRate(sampleRate);
}

void SamplerEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    sampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    float level = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        level = juce::jmax(level, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    lastLevel.store(level);
}

void SamplerEngine::addSoundFromReader(juce::AudioFormatReader* reader, const juce::String& name)
{
    if (reader == nullptr) return;

    sampler.clearSounds();

    juce::BigInteger midiNotes;
    midiNotes.setRange(0, 128, true);

    sampler.addSound(new juce::SamplerSound(
        name,
        *reader,
        midiNotes,
        rootNote,
        0.001,
        0.1,
        20.0
    ));

    currentSampleName = name;
    sampleLoaded = true;
    sampler.setCurrentPlaybackSampleRate(storedSampleRate);

    delete reader;
}

void SamplerEngine::loadSample(const juce::File& wavFile)
{
    // Read file into memory so we own the data
    juce::FileInputStream fis(wavFile);
    if (!fis.openedOk()) return;

    currentSampleData.reset();
    currentSampleData.setSize((size_t)fis.getTotalLength());
    fis.read(currentSampleData.getData(), (int)fis.getTotalLength());

    auto* stream = new juce::MemoryInputStream(currentSampleData, false);
    auto* reader = formatManager.createReaderFor(std::unique_ptr<juce::InputStream>(stream));
    addSoundFromReader(reader, wavFile.getFileNameWithoutExtension());
}

void SamplerEngine::loadSampleFromMemory(const void* data, size_t size, const juce::String& name)
{
    currentSampleData.reset();
    currentSampleData.append(data, size);

    auto* stream = new juce::MemoryInputStream(currentSampleData, false);
    auto* reader = formatManager.createReaderFor(std::unique_ptr<juce::InputStream>(stream));
    addSoundFromReader(reader, name);
}
