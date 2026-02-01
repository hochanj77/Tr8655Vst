#include "VUMeter.h"
#include "BinaryData.h"

VUMeter::VUMeter(const juce::String& labelText)
    : label(labelText)
{
    frameImage = juce::ImageFileFormat::loadFrom(
        BinaryData::meterframe_png, BinaryData::meterframe_pngSize);
    startTimerHz(60);
}

void VUMeter::setLevel(float newLevel)
{
    currentLevel = juce::jlimit(0.0f, 1.0f, newLevel);
}

void VUMeter::timerCallback()
{
    if (currentLevel > displayLevel)
        displayLevel += (currentLevel - displayLevel) * 0.6f;
    else
        displayLevel += (currentLevel - displayLevel) * 0.04f;

    if (displayLevel > peakLevel)
    {
        peakLevel = displayLevel;
        peakHoldCounter = peakHoldFrames;
    }
    else if (peakHoldCounter > 0)
        peakHoldCounter--;
    else
    {
        peakLevel -= 0.008f;
        if (peakLevel < 0.0f) peakLevel = 0.0f;
    }

    repaint();
}

juce::Colour VUMeter::getSegmentOnColour(int index) const
{
    if (index >= 20) return juce::Colour(0xffef4444);
    if (index >= 15) return juce::Colour(0xfff59e0b);
    return juce::Colour(0xff10b981);
}

juce::Colour VUMeter::getSegmentOffColour(int index) const
{
    if (index >= 20) return juce::Colour(0xff7f1d1d);
    if (index >= 15) return juce::Colour(0xff713f12);
    return juce::Colour(0xff064e3b);
}

juce::Colour VUMeter::getSegmentGlowColour(int index) const
{
    if (index >= 20) return juce::Colour(0x80ef4444);
    if (index >= 15) return juce::Colour(0x66f59e0b);
    return juce::Colour(0x6610b981);
}

void VUMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float meterW = bounds.getWidth();
    float meterH = bounds.getHeight() - 14.0f; // leave room for label below

    // Draw meter frame PNG as background
    if (frameImage.isValid())
    {
        g.drawImage(frameImage, 0, 0, (int)meterW, (int)meterH,
                    0, 0, frameImage.getWidth(), frameImage.getHeight());
    }

    // Calculate segment dimensions to fit inside the frame
    float padding = 2.0f;
    float segAreaX = padding;
    float segAreaW = meterW - padding * 2;
    float segAreaTop = padding;
    float segAreaBottom = meterH - padding;
    float segAreaH = segAreaBottom - segAreaTop;

    float segGap = 1.5f;
    float totalGaps = (numSegments - 1) * segGap;
    float segH = (segAreaH - totalGaps) / (float)numSegments;

    int litSegments = (int)(displayLevel * numSegments);
    int peakSegment = juce::jlimit(0, numSegments - 1, (int)(peakLevel * numSegments));

    for (int i = 0; i < numSegments; ++i)
    {
        float segY = segAreaBottom - (i + 1) * segH - i * segGap;
        auto segRect = juce::Rectangle<float>(segAreaX, segY, segAreaW, segH);

        bool isLit = i < litSegments;
        bool isPeak = (i == peakSegment && peakHoldCounter > 0);

        if (isLit || isPeak)
        {
            auto onColour = getSegmentOnColour(i);
            auto glowColour = getSegmentGlowColour(i);

            g.setColour(glowColour.withAlpha(isPeak && !isLit ? 0.3f : 0.4f));
            g.fillRoundedRectangle(segRect.expanded(1.0f), 0.5f);

            float alpha = isLit ? 1.0f : 0.8f;
            g.setColour(onColour.withAlpha(alpha));
        }
        else
        {
            g.setColour(getSegmentOffColour(i).withAlpha(0.15f));
        }

        g.fillRoundedRectangle(segRect, 0.5f);
    }

    // Label below
    if (label.isNotEmpty())
    {
        g.setColour(juce::Colour(0xff737373));
        g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 7.0f, juce::Font::bold));
        auto labelArea = juce::Rectangle<float>(0, meterH + 2, bounds.getWidth(), 10);
        g.drawText(label.toUpperCase(), labelArea, juce::Justification::centred, false);
    }
}
