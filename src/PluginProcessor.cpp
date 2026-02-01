#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

TR8655AudioProcessor::TR8655AudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Load first preset on startup
    loadPreset(0);
}

TR8655AudioProcessor::~TR8655AudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout TR8655AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"gain", 1}, "Gain",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 40.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"drive", 1}, "Drive",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 25.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"sub", 1}, "Sub",
        juce::NormalisableRange<float>(20.0f, 120.0f, 0.1f), 75.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"punch", 1}, "Punch",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 65.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"thresh", 1}, "Thresh",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"ratio", 1}, "Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f), 4.0f,
        juce::AudioParameterFloatAttributes().withLabel(":1")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mix", 1}, "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"output", 1}, "Output",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    return { params.begin(), params.end() };
}

void TR8655AudioProcessor::loadPreset(int presetIndex)
{
    auto presets = getPresetList();
    if (presetIndex < 0 || presetIndex >= (int)presets.size()) return;

    currentPresetIndex = presetIndex;
    customSamplePath.clear();
    const auto& preset = presets[(size_t)presetIndex];

    // Load sample from binary data
    int dataSize = 0;
    const char* data = BinaryData::getNamedResource(preset.sampleResourceName.toRawUTF8(), dataSize);
    if (data != nullptr && dataSize > 0)
    {
        samplerEngine.setRootNote(preset.rootNote);
        samplerEngine.loadSampleFromMemory(data, (size_t)dataSize, preset.name);
    }

    // Set effect parameters
    if (auto* p = apvts.getParameter("gain"))   p->setValueNotifyingHost(p->convertTo0to1(preset.gain));
    if (auto* p = apvts.getParameter("drive"))  p->setValueNotifyingHost(p->convertTo0to1(preset.drive));
    if (auto* p = apvts.getParameter("sub"))    p->setValueNotifyingHost(p->convertTo0to1(preset.sub));
    if (auto* p = apvts.getParameter("punch"))  p->setValueNotifyingHost(p->convertTo0to1(preset.punch));
    if (auto* p = apvts.getParameter("thresh")) p->setValueNotifyingHost(p->convertTo0to1(preset.thresh));
    if (auto* p = apvts.getParameter("ratio"))  p->setValueNotifyingHost(p->convertTo0to1(preset.ratio));
    if (auto* p = apvts.getParameter("mix"))    p->setValueNotifyingHost(p->convertTo0to1(preset.mix));
    if (auto* p = apvts.getParameter("output")) p->setValueNotifyingHost(p->convertTo0to1(preset.output));
}

void TR8655AudioProcessor::loadCustomSample(const juce::File& wavFile)
{
    if (!wavFile.existsAsFile()) return;
    customSamplePath = wavFile.getFullPathName();
    currentPresetIndex = -1;
    samplerEngine.loadSample(wavFile);
}

const juce::String TR8655AudioProcessor::getName() const { return JucePlugin_Name; }
bool TR8655AudioProcessor::acceptsMidi() const { return true; }
bool TR8655AudioProcessor::producesMidi() const { return false; }
bool TR8655AudioProcessor::isMidiEffect() const { return false; }
double TR8655AudioProcessor::getTailLengthSeconds() const { return 0.5; }
int TR8655AudioProcessor::getNumPrograms() { return getPresetCount(); }
int TR8655AudioProcessor::getCurrentProgram() { return currentPresetIndex >= 0 ? currentPresetIndex : 0; }
void TR8655AudioProcessor::setCurrentProgram(int index) { loadPreset(index); }
const juce::String TR8655AudioProcessor::getProgramName(int index)
{
    auto presets = getPresetList();
    if (index >= 0 && index < (int)presets.size()) return presets[(size_t)index].name;
    return {};
}
void TR8655AudioProcessor::changeProgramName(int, const juce::String&) {}

void TR8655AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    samplerEngine.prepareToPlay(sampleRate, samplesPerBlock);

    currentSpec.sampleRate = sampleRate;
    currentSpec.maximumBlockSize = (juce::uint32)samplesPerBlock;
    currentSpec.numChannels = 2;

    // Sub filter setup
    auto subCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 75.0, 0.7, 1.0);
    subFilterL.coefficients = subCoeffs;
    subFilterR.coefficients = subCoeffs;
    subFilterL.reset();
    subFilterR.reset();

    envelopeFollower = 0.0f;
    prevSample = 0.0f;
}

void TR8655AudioProcessor::releaseResources() {}

bool TR8655AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    auto outSet = layouts.getMainOutputChannelSet();
    return outSet == juce::AudioChannelSet::mono() || outSet == juce::AudioChannelSet::stereo();
}

void TR8655AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Track MIDI activity
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            lastMidiNote.store(msg.getNoteNumber());
            midiActivity.store(true);
        }
    }

    // Clear buffer (we're a synth)
    buffer.clear();

    if (!engaged.load()) return;

    // Render sampler
    samplerEngine.processBlock(buffer, midiMessages);

    // Measure input level (post-sampler, pre-effects)
    float inLevel = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        inLevel = juce::jmax(inLevel, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    inputLevel.store(inLevel);

    // Apply effects
    applyEffectsChain(buffer);

    // Measure output level
    float outLevel = 0.0f;
    float bassLvl = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        outLevel = juce::jmax(outLevel, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    outputLevel.store(outLevel);

    // Simple bass level estimate (low pass energy)
    if (buffer.getNumSamples() > 0)
    {
        float sum = 0.0f;
        auto* data = buffer.getReadPointer(0);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float filtered = data[i] * 0.1f + prevSample * 0.9f;
            sum += std::abs(filtered);
            prevSample = filtered;
        }
        bassLvl = sum / (float)buffer.getNumSamples() * 4.0f;
    }
    bassLevel.store(juce::jlimit(0.0f, 1.0f, bassLvl));
}

void TR8655AudioProcessor::applyEffectsChain(juce::AudioBuffer<float>& buffer)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    float gainVal  = apvts.getRawParameterValue("gain")->load();
    float driveVal = apvts.getRawParameterValue("drive")->load();
    float subVal   = apvts.getRawParameterValue("sub")->load();
    float punchVal = apvts.getRawParameterValue("punch")->load();
    float threshVal= apvts.getRawParameterValue("thresh")->load();
    float ratioVal = apvts.getRawParameterValue("ratio")->load();
    float mixVal   = apvts.getRawParameterValue("mix")->load();
    float outVal   = apvts.getRawParameterValue("output")->load();

    // Store dry signal for mix
    juce::AudioBuffer<float> dryBuffer;
    if (mixVal < 100.0f)
    {
        dryBuffer.makeCopyOf(buffer);
    }

    // 1. Input Gain
    float gainLinear = std::pow(10.0f, (gainVal - 50.0f) / 40.0f);
    buffer.applyGain(gainLinear);

    // 2. Saturation/Drive
    float driveAmount = driveVal / 100.0f;
    if (driveAmount > 0.001f)
    {
        float driveScale = 1.0f + driveAmount * 4.0f;
        float normFactor = 1.0f / std::tanh(driveScale);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
                data[i] = std::tanh(data[i] * driveScale) * normFactor;
        }
    }

    // 3. Sub Enhancement (low shelf filter)
    float subBoostDb = juce::jmap(subVal, 20.0f, 120.0f, 0.0f, 12.0f);
    float subGainLinear = std::pow(10.0f, subBoostDb / 20.0f);
    auto subCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        currentSpec.sampleRate, subVal, 0.7, (double)subGainLinear);
    subFilterL.coefficients = subCoeffs;
    subFilterR.coefficients = subCoeffs;

    if (numChannels > 0)
    {
        auto* dataL = buffer.getWritePointer(0);
        for (int i = 0; i < numSamples; ++i)
            dataL[i] = subFilterL.processSample(dataL[i]);
    }
    if (numChannels > 1)
    {
        auto* dataR = buffer.getWritePointer(1);
        for (int i = 0; i < numSamples; ++i)
            dataR[i] = subFilterR.processSample(dataR[i]);
    }

    // 4. Compression
    float threshDb = juce::jmap(threshVal, 0.0f, 100.0f, 0.0f, -40.0f);
    float threshLinear = std::pow(10.0f, threshDb / 20.0f);
    float compRatio = ratioVal;
    float attackCoeff = std::exp(-1.0f / (float)(currentSpec.sampleRate * 0.001));
    float releaseCoeff = std::exp(-1.0f / (float)(currentSpec.sampleRate * 0.1));

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float absVal = std::abs(data[i]);
            float coeff = (absVal > envelopeFollower) ? attackCoeff : releaseCoeff;
            envelopeFollower = coeff * envelopeFollower + (1.0f - coeff) * absVal;

            if (envelopeFollower > threshLinear && threshLinear > 0.0001f)
            {
                float dbOver = 20.0f * std::log10(envelopeFollower / threshLinear);
                float dbReduction = dbOver * (1.0f - 1.0f / compRatio);
                float gainReduction = std::pow(10.0f, -dbReduction / 20.0f);
                data[i] *= gainReduction;
            }
        }
    }

    // 5. Punch (Transient Shaper)
    float punchBoostDb = juce::jmap(punchVal, 0.0f, 100.0f, 0.0f, 6.0f);
    float punchGain = std::pow(10.0f, punchBoostDb / 20.0f);
    int punchSamples = (int)(currentSpec.sampleRate * 0.015); // 15ms window

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        float localEnv = 0.0f;
        for (int i = 0; i < numSamples; ++i)
        {
            float absVal = std::abs(data[i]);
            float attackRate = 0.99f;
            float releaseRate = 0.9995f;
            float prevEnv = localEnv;
            localEnv = (absVal > localEnv)
                ? attackRate * localEnv + (1.0f - attackRate) * absVal
                : releaseRate * localEnv;

            // Transient = when signal rises above envelope
            if (absVal > prevEnv * 1.2f)
                data[i] *= punchGain;
        }
    }

    // 6. Wet/Dry Mix
    float wetMix = mixVal / 100.0f;
    if (wetMix < 1.0f && dryBuffer.getNumSamples() == numSamples)
    {
        float dryMix = 1.0f - wetMix;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* wet = buffer.getWritePointer(ch);
            auto* dry = dryBuffer.getReadPointer(ch);
            for (int i = 0; i < numSamples; ++i)
                wet[i] = wet[i] * wetMix + dry[i] * dryMix;
        }
    }

    // 7. Output Gain
    float outputLinear = std::pow(10.0f, outVal / 20.0f);
    buffer.applyGain(outputLinear);
}

bool TR8655AudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* TR8655AudioProcessor::createEditor()
{
    return new TR8655AudioProcessorEditor(*this);
}

void TR8655AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty("presetIndex", currentPresetIndex, nullptr);
    state.setProperty("customSamplePath", customSamplePath, nullptr);
    state.setProperty("engaged", engaged.load(), nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TR8655AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));

        int presetIdx = apvts.state.getProperty("presetIndex", 0);
        juce::String customPath = apvts.state.getProperty("customSamplePath", "").toString();
        bool eng = apvts.state.getProperty("engaged", true);
        engaged.store(eng);

        if (customPath.isNotEmpty())
        {
            juce::File f(customPath);
            if (f.existsAsFile())
                loadCustomSample(f);
            else
                loadPreset(0);
        }
        else
        {
            loadPreset(presetIdx);
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TR8655AudioProcessor();
}
