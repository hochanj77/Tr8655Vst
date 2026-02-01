#include "Visualizer.h"
#include "BinaryData.h"

Visualizer::Visualizer()
{
    sphereImage = juce::ImageFileFormat::loadFrom(
        BinaryData::visualizersphere_png, BinaryData::visualizersphere_pngSize);
    startTimerHz(60);
}

void Visualizer::timerCallback()
{
    time += 0.05f;
    intensity += (targetIntensity - intensity) * 0.12f;
    bassIntensity += (targetBassIntensity - bassIntensity) * 0.08f;
    repaint();
}

void Visualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto centre = bounds.getCentre();
    float size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    float radius = size / 2.0f;

    // Draw visualizer-sphere.png as background
    if (sphereImage.isValid())
    {
        g.drawImage(sphereImage,
                    centre.x - radius, centre.y - radius, size, size,
                    0, 0, sphereImage.getWidth(), sphereImage.getHeight());
    }

    // Clip waves to circle
    {
        juce::Path clipPath;
        clipPath.addEllipse(centre.x - radius, centre.y - radius, size, size);
        g.saveState();
        g.reduceClipRegion(clipPath);
    }

    // Draw 4 waves with specified colors
    auto waveBounds = juce::Rectangle<float>(centre.x - radius, centre.y - radius, size, size);
    drawWaves(g, waveBounds);

    g.restoreState();

    // Drop highlight overlay
    if (dropHighlight)
    {
        g.setColour(juce::Colour(0x333B82F6));
        g.fillEllipse(centre.x - radius, centre.y - radius, size, size);
        g.setColour(juce::Colour(0xff60A5FA));
        g.setFont(juce::FontOptions(14.0f));
        g.drawText("DROP WAV TO LOAD", bounds, juce::Justification::centred, false);
    }
}

void Visualizer::drawWaves(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    float centerY = bounds.getCentreY();

    // Wave 1: #2542DD filled, slow
    drawWaveLayer(g, bounds, juce::Colour(0xff2542DD), 0.02f, 0.04f, 1.5f, 1.0f, 0.35f, true, 0.0f);

    // Wave 2: #3E36D4 filled, reverse
    drawWaveLayer(g, bounds, juce::Colour(0xff3E36D4), -0.015f, 0.06f, 1.2f, 1.5f, 0.25f, true, 1.0f);

    // Wave 3: #FFFFFF stroked, main center line
    drawWaveLayer(g, bounds, juce::Colour(0xffffffff), 0.04f, 0.05f, 1.2f, 1.2f, 0.8f, false, 2.0f);

    // Wave 4: #5B54DB stroked, detail
    drawWaveLayer(g, bounds, juce::Colour(0xff5B54DB), -0.03f, 0.08f, 0.9f, 1.8f, 0.6f, false, 3.0f);

    // White center line with glow
    {
        float lineWidth = 5.0f + bassIntensity * 10.0f;
        float lineAlpha = 0.8f + intensity * 0.2f;

        g.setColour(juce::Colours::white.withAlpha(lineAlpha * 0.15f));
        g.drawLine(bounds.getX(), centerY, bounds.getRight(), centerY, lineWidth + 8.0f);

        g.setColour(juce::Colours::white.withAlpha(lineAlpha * 0.3f));
        g.drawLine(bounds.getX(), centerY, bounds.getRight(), centerY, lineWidth + 4.0f);

        g.setColour(juce::Colours::white.withAlpha(lineAlpha));
        g.drawLine(bounds.getX(), centerY, bounds.getRight(), centerY, lineWidth);
    }
}

void Visualizer::drawWaveLayer(juce::Graphics& g, juce::Rectangle<float> bounds,
                                juce::Colour color, float speed, float freq,
                                float amplitudeMultiplier, float warpIntensity,
                                float opacity, bool isFilled, float indexOffset)
{
    int points = 70;
    float size = bounds.getWidth();
    float centerY = bounds.getCentreY();
    float step = size / (float)(points - 1);

    float effectiveOpacity = opacity * (0.8f + intensity * 0.2f);

    juce::Path wavePath;
    bool started = false;

    for (int i = 0; i < points; ++i)
    {
        float x = (float)i * step;
        float normalizedX = ((float)i / (float)(points - 1)) * 2.0f - 1.0f;
        float edgeFade = std::pow(1.0f - std::pow(normalizedX, 2.0f), 1.1f);

        float t = time * speed * (0.8f + intensity * 2.0f);
        float primaryWave = std::sin((float)i * freq + t) * (40.0f + intensity * 120.0f * amplitudeMultiplier);
        float warp = std::sin((float)i * 0.08f + t * 0.4f) * warpIntensity * 30.0f * bassIntensity;
        float secondaryWave = std::sin((float)i * freq * 1.5f - t * 0.8f) * (25.0f + bassIntensity * 60.0f);

        float y = centerY + (primaryWave + secondaryWave + warp) * edgeFade;
        float drawX = bounds.getX() + x;

        if (!started) { wavePath.startNewSubPath(drawX, y); started = true; }
        else wavePath.lineTo(drawX, y);
    }

    if (isFilled)
    {
        wavePath.lineTo(bounds.getRight(), centerY);
        wavePath.lineTo(bounds.getX(), centerY);
        wavePath.closeSubPath();

        g.setColour(color.withAlpha(effectiveOpacity));
        g.fillPath(wavePath);
    }
    else
    {
        float strokeW = (10.0f + indexOffset * 1.5f) * (1.0f + bassIntensity * 0.8f);

        g.setColour(color.withAlpha(effectiveOpacity * 0.15f));
        g.strokePath(wavePath, juce::PathStrokeType(strokeW + 4.0f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.setColour(color.withAlpha(effectiveOpacity * 0.3f));
        g.strokePath(wavePath, juce::PathStrokeType(strokeW + 2.0f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.setColour(color.withAlpha(effectiveOpacity));
        g.strokePath(wavePath, juce::PathStrokeType(strokeW * 0.3f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}
