#include "CustomKnob.h"
#include "BinaryData.h"

CustomKnob::CustomKnob(const juce::String& labelText)
    : label(labelText)
{
    setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    setRotaryParameters(juce::degreesToRadians(225.0f),
                        juce::degreesToRadians(495.0f), true);

    knobImage = juce::ImageFileFormat::loadFrom(
        BinaryData::knobcomplete_png, BinaryData::knobcomplete_pngSize);
}

void CustomKnob::resized() {}

void CustomKnob::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Knob image area: 56x56 at top center
    float knobSize = 56.0f;
    float knobX = (bounds.getWidth() - knobSize) / 2.0f;
    float knobY = 0.0f;
    float cx = knobX + knobSize / 2.0f;
    float cy = knobY + knobSize / 2.0f;

    // Calculate rotation angle from parameter value
    float normValue = (float)valueToProportionOfLength(getValue());
    float angle = juce::jmap(normValue, 0.0f, 1.0f,
        juce::degreesToRadians(-135.0f), juce::degreesToRadians(135.0f));

    // Draw rotated knob PNG
    if (knobImage.isValid())
    {
        float imgW = (float)knobImage.getWidth();
        float imgH = (float)knobImage.getHeight();
        float scale = knobSize / juce::jmax(imgW, imgH);

        auto transform = juce::AffineTransform()
            .translated(-imgW / 2.0f, -imgH / 2.0f)
            .scaled(scale)
            .rotated(angle)
            .translated(cx, cy);

        g.drawImageTransformed(knobImage, transform, false);
    }

    // Label below knob
    auto labelBounds = juce::Rectangle<float>(0, knobSize + 4, bounds.getWidth(), 12);
    drawLabel(g, labelBounds);

    // Value box below label
    auto valueBounds = juce::Rectangle<float>((bounds.getWidth() - 50) / 2.0f,
                                               knobSize + 18, 50, 17);
    drawValueBox(g, valueBounds);
}

void CustomKnob::drawLabel(juce::Graphics& g, juce::Rectangle<float> area)
{
    g.setColour(juce::Colour(0xff737373));
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::bold));
    g.drawText(label.toUpperCase(), area, juce::Justification::centred, false);
}

void CustomKnob::drawValueBox(juce::Graphics& g, juce::Rectangle<float> area)
{
    // bg-[#08080a] border border-white/5
    g.setColour(juce::Colour(0xff08080a));
    g.fillRoundedRectangle(area, 3.0f);

    // Inner shadow at top
    juce::ColourGradient insetShadow(juce::Colour(0x30000000), area.getCentreX(), area.getY(),
                                      juce::Colour(0x00000000), area.getCentreX(), area.getY() + 4.0f, false);
    g.setGradientFill(insetShadow);
    g.fillRoundedRectangle(area, 3.0f);

    g.setColour(juce::Colour(0x0dffffff));
    g.drawRoundedRectangle(area, 3.0f, 0.75f);

    // Value text: text-[10px] font-mono text-blue-400/90
    g.setColour(juce::Colour(0xe660A5FA));
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain));

    juce::String valueText;
    if (valueSuffix == "dB" && getRange().getStart() < 0)
        valueText = juce::String(getValue(), 1) + " " + valueSuffix;
    else if (valueSuffix == ":1")
        valueText = juce::String((int)getValue()) + valueSuffix;
    else
        valueText = juce::String((int)getValue()) + " " + valueSuffix;

    g.drawText(valueText, area, juce::Justification::centred, false);
}
