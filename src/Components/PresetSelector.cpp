#include "PresetSelector.h"
#include "BinaryData.h"

PresetSelector::PresetSelector()
{
    boxImage = juce::ImageFileFormat::loadFrom(
        BinaryData::presetbox_png, BinaryData::presetbox_pngSize);

    leftButton.setButtonText("<");
    rightButton.setButtonText(">");

    leftButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    rightButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    leftButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffa3a3a3));
    rightButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffa3a3a3));

    leftButton.onClick = [this] { prevPreset(); };
    rightButton.onClick = [this] { nextPreset(); };

    addAndMakeVisible(leftButton);
    addAndMakeVisible(rightButton);
}

void PresetSelector::setPresetName(const juce::String& name)
{
    currentPresetName = name;
    repaint();
}

void PresetSelector::setPresetList(const juce::StringArray& names)
{
    presetNames = names;
}

void PresetSelector::setCurrentIndex(int index)
{
    if (presetNames.isEmpty()) return;
    currentIndex = juce::jlimit(0, presetNames.size() - 1, index);
    currentPresetName = presetNames[currentIndex];
    repaint();
}

void PresetSelector::prevPreset()
{
    if (presetNames.isEmpty()) return;
    currentIndex = (currentIndex - 1 + presetNames.size()) % presetNames.size();
    currentPresetName = presetNames[currentIndex];
    repaint();
    if (onPresetChanged) onPresetChanged(currentIndex);
}

void PresetSelector::nextPreset()
{
    if (presetNames.isEmpty()) return;
    currentIndex = (currentIndex + 1) % presetNames.size();
    currentPresetName = presetNames[currentIndex];
    repaint();
    if (onPresetChanged) onPresetChanged(currentIndex);
}

void PresetSelector::resized()
{
    auto bounds = getLocalBounds();
    auto lower = bounds.withTrimmedTop(14);
    leftButton.setBounds(lower.removeFromLeft(24));
    rightButton.setBounds(lower.removeFromRight(24));
}

void PresetSelector::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // "PATCH MEMORY" label
    g.setColour(juce::Colour(0xff737373));
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 8.0f, juce::Font::bold));
    g.drawText("PATCH MEMORY", bounds.removeFromTop(14), juce::Justification::centred, false);

    // Draw preset-box.png as background for the display area
    auto displayBounds = bounds.reduced(22, 0);
    if (boxImage.isValid())
    {
        g.drawImage(boxImage,
                    (int)displayBounds.getX(), (int)displayBounds.getY(),
                    (int)displayBounds.getWidth(), (int)displayBounds.getHeight(),
                    0, 0, boxImage.getWidth(), boxImage.getHeight());
    }

    // Preset name: text-[11px] font-mono font-bold text-blue-500
    g.setColour(juce::Colour(0xff3B82F6));
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::bold));
    g.drawText(currentPresetName, displayBounds, juce::Justification::centred, false);
}
