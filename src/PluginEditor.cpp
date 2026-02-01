#include "PluginEditor.h"
#include "BinaryData.h"

// ============================================================================
// EngageButton — uses engage-on.png / engage-off.png
// ============================================================================

EngageButton::EngageButton()
{
    onImage = juce::ImageFileFormat::loadFrom(
        BinaryData::engageon_png, BinaryData::engageon_pngSize);
    offImage = juce::ImageFileFormat::loadFrom(
        BinaryData::engageoff_png, BinaryData::engageoff_pngSize);
}

void EngageButton::paint(juce::Graphics& g)
{
    auto& img = active ? onImage : offImage;
    if (img.isValid())
    {
        g.drawImage(img, getLocalBounds().toFloat(),
                    juce::RectanglePlacement::stretchToFit);
    }
}

void EngageButton::mouseDown(const juce::MouseEvent&)
{
    active = !active;
    repaint();
    if (onToggle) onToggle(active);
}

// ============================================================================
// TR8655AudioProcessorEditor
// ============================================================================

TR8655AudioProcessorEditor::TR8655AudioProcessorEditor(TR8655AudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(900, 600);

    logoImage = juce::ImageFileFormat::loadFrom(
        BinaryData::logoheader_png, BinaryData::logoheader_pngSize);

    gainKnob.setValueSuffix("dB");
    driveKnob.setValueSuffix("%");
    threshKnob.setValueSuffix("dB");
    ratioKnob.setValueSuffix(":1");
    subKnob.setValueSuffix("Hz");
    punchKnob.setValueSuffix("%");
    mixKnob.setValueSuffix("%");
    outputKnob.setValueSuffix("dB");

    for (auto* k : { &gainKnob, &driveKnob, &threshKnob, &ratioKnob,
                     &subKnob, &punchKnob, &mixKnob, &outputKnob })
        addAndMakeVisible(k);

    gainAtt   = std::make_unique<Attachment>(processorRef.apvts, "gain",   gainKnob);
    driveAtt  = std::make_unique<Attachment>(processorRef.apvts, "drive",  driveKnob);
    threshAtt = std::make_unique<Attachment>(processorRef.apvts, "thresh", threshKnob);
    ratioAtt  = std::make_unique<Attachment>(processorRef.apvts, "ratio",  ratioKnob);
    subAtt    = std::make_unique<Attachment>(processorRef.apvts, "sub",    subKnob);
    punchAtt  = std::make_unique<Attachment>(processorRef.apvts, "punch",  punchKnob);
    mixAtt    = std::make_unique<Attachment>(processorRef.apvts, "mix",    mixKnob);
    outputAtt = std::make_unique<Attachment>(processorRef.apvts, "output", outputKnob);

    addAndMakeVisible(inMeter);
    addAndMakeVisible(subMeter);
    addAndMakeVisible(visualizer);
    addAndMakeVisible(presetSelector);

    juce::StringArray presetNames;
    for (auto& preset : getPresetList())
        presetNames.add(preset.name);
    presetSelector.setPresetList(presetNames);
    presetSelector.setCurrentIndex(processorRef.getCurrentPresetIndex());
    presetSelector.onPresetChanged = [this](int index) {
        processorRef.loadPreset(index);
    };

    addAndMakeVisible(engageButton);
    engageButton.setActive(processorRef.engaged.load());
    engageButton.onToggle = [this](bool a) { processorRef.engaged.store(a); };

    startTimerHz(60);
}

TR8655AudioProcessorEditor::~TR8655AudioProcessorEditor() { stopTimer(); }

void TR8655AudioProcessorEditor::timerCallback()
{
    inMeter.setLevel(processorRef.inputLevel.load());
    subMeter.setLevel(processorRef.bassLevel.load());
    visualizer.setIntensity(processorRef.outputLevel.load());
    visualizer.setBassIntensity(processorRef.bassLevel.load());
}

// --- Drag & Drop ---
bool TR8655AudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto& f : files)
        if (f.endsWithIgnoreCase(".wav") || f.endsWithIgnoreCase(".aiff"))
            return true;
    return false;
}

void TR8655AudioProcessorEditor::filesDropped(const juce::StringArray& files, int, int)
{
    isFileDragOver = false;
    visualizer.setDropHighlight(false);
    if (files.size() > 0)
    {
        juce::File droppedFile(files[0]);
        if (droppedFile.existsAsFile())
        {
            processorRef.loadCustomSample(droppedFile);
            presetSelector.setPresetName("CUSTOM: " + droppedFile.getFileNameWithoutExtension());
        }
    }
}

void TR8655AudioProcessorEditor::fileDragEnter(const juce::StringArray&, int, int)
{
    isFileDragOver = true;
    visualizer.setDropHighlight(true);
    repaint();
}

void TR8655AudioProcessorEditor::fileDragExit(const juce::StringArray&)
{
    isFileDragOver = false;
    visualizer.setDropHighlight(false);
    repaint();
}

// --- Layout (positions from positions.txt) ---
void TR8655AudioProcessorEditor::resized()
{
    // Knob component size: 76 wide (56 image + 10 padding each side for label),
    // 94 tall (56 knob + 4 gap + 12 label + 4 gap + 18 value box)
    int knobW = 76;
    int knobH = 94;

    // Positions from positions.txt: element X, Y are for the 56x56 knob image
    // Center the wider component on the knob position
    auto placeKnob = [&](juce::Slider& knob, int posX, int posY) {
        knob.setBounds(posX - (knobW - 56) / 2, posY, knobW, knobH);
    };

    // Left panel knobs
    placeKnob(gainKnob,   50,  180);
    placeKnob(driveKnob,  50,  280);
    placeKnob(threshKnob, 120, 180);
    placeKnob(ratioKnob,  120, 280);

    // Right panel knobs
    placeKnob(subKnob,    630, 180);
    placeKnob(punchKnob,  630, 280);
    placeKnob(mixKnob,    700, 180);
    placeKnob(outputKnob, 700, 280);

    // Visualizer: 250, 150, 300, 300
    visualizer.setBounds(250, 150, 300, 300);

    // Meters: 30, 380, 20, 180 and 750, 380, 20, 180
    inMeter.setBounds(30, 380, 20, 194);   // +14 for label below
    subMeter.setBounds(750, 380, 20, 194);

    // Preset selector: 570, 40, 200, 40
    presetSelector.setBounds(570, 30, 200, 54); // +14 for "PATCH MEMORY" label

    // Engage button: 320, 500, 160, 50
    engageButton.setBounds(320, 500, 160, 50);
}

// --- Paint ---
void TR8655AudioProcessorEditor::paint(juce::Graphics& g)
{
    drawBackground(g);
    drawHeader(g);
    drawSectionHeaders(g);
    drawFooter(g);
}

void TR8655AudioProcessorEditor::drawBackground(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float w = bounds.getWidth();
    float h = bounds.getHeight();

    // background: linear-gradient(135deg, #2a2a2e 0%, #1a1a1e 35%, #121215 100%)
    juce::ColourGradient bg(juce::Colour(0xff2a2a2e), 0, 0,
                             juce::Colour(0xff121215), w, h, false);
    bg.addColour(0.35, juce::Colour(0xff1a1a1e));
    g.setGradientFill(bg);
    g.fillAll();

    // Top highlight line
    {
        juce::ColourGradient topLine(juce::Colour(0x00ffffff), 0, 0,
                                      juce::Colour(0x00ffffff), w, 0, false);
        topLine.addColour(0.5, juce::Colour(0x1affffff));
        g.setGradientFill(topLine);
        g.fillRect(0.0f, 0.0f, w, 1.0f);
    }

    // Surface sheen
    {
        juce::ColourGradient sheen(juce::Colour(0x08ffffff), w / 2, 0,
                                    juce::Colour(0x00ffffff), w / 2, h * 0.5f, false);
        g.setGradientFill(sheen);
        g.fillAll();
    }

    // Noise texture at 2% opacity
    juce::Random rng(42);
    g.setColour(juce::Colour(0x05ffffff));
    for (int i = 0; i < 500; ++i)
    {
        float x = rng.nextFloat() * w;
        float y = rng.nextFloat() * h;
        g.fillRect(x, y, 1.0f, 1.0f);
    }

    // Corner screws
    drawScrew(g, 16, 16);
    drawScrew(g, w - 28, 16);
    drawScrew(g, 16, h - 28);
    drawScrew(g, w - 28, h - 28);

    // Panel zones
    float panelWidth = 220.0f;
    float headerH = 130.0f;
    float footerH = 50.0f;

    // Left panel
    g.setColour(juce::Colour(0x1a000000));
    g.fillRect(0.0f, headerH, panelWidth, h - headerH - footerH);
    juce::ColourGradient leftShadow(juce::Colour(0x20000000), panelWidth, headerH,
                                     juce::Colour(0x00000000), panelWidth - 30.0f, headerH, false);
    g.setGradientFill(leftShadow);
    g.fillRect(panelWidth - 30.0f, headerH, 30.0f, h - headerH - footerH);
    g.setColour(juce::Colour(0x66000000));
    g.drawLine(panelWidth, headerH, panelWidth, h - footerH, 1.0f);

    // Right panel
    float rightPanelX = w - panelWidth;
    g.setColour(juce::Colour(0x1a000000));
    g.fillRect(rightPanelX, headerH, panelWidth, h - headerH - footerH);
    juce::ColourGradient rightShadow(juce::Colour(0x20000000), rightPanelX, headerH,
                                      juce::Colour(0x00000000), rightPanelX + 30.0f, headerH, false);
    g.setGradientFill(rightShadow);
    g.fillRect(rightPanelX, headerH, 30.0f, h - headerH - footerH);
    g.setColour(juce::Colour(0x66000000));
    g.drawLine(rightPanelX, headerH, rightPanelX, h - footerH, 1.0f);

    // Header bottom border
    g.setColour(juce::Colour(0x66000000));
    g.drawLine(0, headerH, w, headerH, 1.0f);

    // Outer border
    g.setColour(juce::Colour(0x0dffffff));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);
}

void TR8655AudioProcessorEditor::drawScrew(juce::Graphics& g, float x, float y)
{
    float sz = 12.0f;
    auto sb = juce::Rectangle<float>(x, y, sz, sz);
    auto sc = sb.getCentre();

    g.setColour(juce::Colour(0xff262626));
    g.fillEllipse(sb);

    juce::ColourGradient shadow(juce::Colour(0xaa000000), sc.x, sb.getY(),
                                 juce::Colour(0x00000000), sc.x, sc.y, false);
    g.setGradientFill(shadow);
    g.fillEllipse(sb);

    g.setColour(juce::Colour(0x10ffffff));
    juce::Path arc;
    arc.addCentredArc(sc.x, sc.y, sz / 2, sz / 2, 0.0f,
                       juce::degreesToRadians(30.0f), juce::degreesToRadians(150.0f), true);
    g.strokePath(arc, juce::PathStrokeType(0.75f));

    g.setColour(juce::Colour(0xff111111));
    float sl = 7.0f;
    float a = juce::degreesToRadians(45.0f);
    float dx = std::cos(a) * sl / 2;
    float dy = std::sin(a) * sl / 2;
    g.drawLine(sc.x - dx, sc.y - dy, sc.x + dx, sc.y + dy, 1.2f);
}

void TR8655AudioProcessorEditor::drawHeader(juce::Graphics& g)
{
    // Logo image: 30, 30, 240, 60 from positions.txt
    if (logoImage.isValid())
    {
        g.drawImage(logoImage, 30, 30, 240, 60,
                    0, 0, logoImage.getWidth(), logoImage.getHeight());
    }
}

void TR8655AudioProcessorEditor::drawSectionHeaders(juce::Graphics& g)
{
    auto drawSection = [&](float x, float y, const juce::String& text, int iconType) {
        g.setColour(juce::Colour(0xff3B82F6));
        float iconSize = 12.0f;
        switch (iconType)
        {
            case 0: drawIconSignal(g, x, y + 1, iconSize); break;
            case 1: drawIconDynamics(g, x, y + 1, iconSize); break;
            case 2: drawIconEqualizer(g, x, y + 1, iconSize); break;
            case 3: drawIconMaster(g, x, y + 1, iconSize); break;
        }

        g.setColour(juce::Colour(0xffa3a3a3));
        g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::bold));
        g.drawText(text, juce::Rectangle<float>(x + 18, y, 160, 14), juce::Justification::centredLeft, false);

        g.setColour(juce::Colour(0x0dffffff));
        g.drawLine(x, y + 20, x + 160, y + 20, 0.75f);
    };

    // Section headers above knob groups
    drawSection(40, 155, "SIGNAL", 0);
    drawSection(40, 255, "DYNAMICS", 1);
    drawSection(622, 155, "EQUALIZER", 2);
    drawSection(622, 255, "MASTER", 3);
}

void TR8655AudioProcessorEditor::drawFooter(juce::Graphics& g)
{
    float w = (float)getWidth();
    float footerY = 550.0f;

    g.setColour(juce::Colour(0x99000000));
    g.fillRect(0.0f, footerY, w, 50.0f);
    g.setColour(juce::Colour(0x66000000));
    g.drawLine(0, footerY, w, footerY, 1.0f);

    float textY = footerY + 18;
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::plain));

    // Sample rate
    g.setColour(juce::Colour(0xff10b981));
    g.fillEllipse(60, textY, 6, 6);
    g.setColour(juce::Colour(0xff737373));
    double sr = processorRef.getSampleRate();
    juce::String srText = juce::String((int)(sr / 1000.0)) + " KHZ";
    g.drawText(srText, 72, (int)textY - 2, 60, 14, juce::Justification::centredLeft, false);

    // Bit depth
    g.setColour(juce::Colour(0xff10b981));
    g.fillEllipse(145, textY, 6, 6);
    g.setColour(juce::Colour(0xff737373));
    g.drawText("32-BIT", 157, (int)textY - 2, 50, 14, juce::Justification::centredLeft, false);

    // CPU bar
    g.setColour(juce::Colour(0xff525252));
    g.drawText("CPU", 700, (int)textY - 2, 30, 14, juce::Justification::centredLeft, false);

    float barX = 735;
    float barW = 96;
    float barH = 6;
    float barY = textY;
    g.setColour(juce::Colour(0xff0a0a0a));
    g.fillRoundedRectangle(barX, barY, barW, barH, 3.0f);
    g.setColour(juce::Colour(0x0dffffff));
    g.drawRoundedRectangle(barX, barY, barW, barH, 3.0f, 0.5f);

    float cpuLoad = juce::jlimit(0.0f, 1.0f, processorRef.outputLevel.load() * 0.3f);
    float fillW = juce::jmax(2.0f, cpuLoad * barW);
    juce::ColourGradient cpuGrad(juce::Colour(0xff2563eb), barX, barY,
                                  juce::Colour(0xff60a5fa), barX + fillW, barY, false);
    g.setGradientFill(cpuGrad);
    g.fillRoundedRectangle(barX, barY, fillW, barH, 3.0f);

    // MIDI note
    int lastNote = processorRef.lastMidiNote.load();
    if (lastNote >= 0)
    {
        static const char* noteNames[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
        juce::String noteName = juce::String(noteNames[lastNote % 12]) + juce::String(lastNote / 12 - 1);
        g.setColour(juce::Colour(0xff60A5FA));
        g.drawText(noteName, 845, (int)textY - 2, 40, 14, juce::Justification::centredLeft, false);
    }
}

// ============================================================================
// Section Icons
// ============================================================================

void TR8655AudioProcessorEditor::drawIconSignal(juce::Graphics& g, float x, float y, float s)
{
    g.setColour(juce::Colour(0xff3B82F6));
    juce::Path p;
    float sc = s / 24.0f;
    p.startNewSubPath(x + 13 * sc, y + 2 * sc);
    p.lineTo(x + 3 * sc, y + 14 * sc);
    p.lineTo(x + 12 * sc, y + 14 * sc);
    p.lineTo(x + 11 * sc, y + 22 * sc);
    p.lineTo(x + 21 * sc, y + 10 * sc);
    p.lineTo(x + 12 * sc, y + 10 * sc);
    p.closeSubPath();
    g.fillPath(p);
}

void TR8655AudioProcessorEditor::drawIconDynamics(juce::Graphics& g, float x, float y, float s)
{
    g.setColour(juce::Colour(0xff3B82F6));
    float sc = s / 24.0f;
    auto drawWaveLine = [&](float cy) {
        juce::Path p;
        p.startNewSubPath(x + 2 * sc, cy);
        p.cubicTo(x + 5 * sc, cy - 3 * sc, x + 7 * sc, cy - 3 * sc, x + 9.5f * sc, cy);
        p.cubicTo(x + 12 * sc, cy + 3 * sc, x + 14 * sc, cy + 3 * sc, x + 16.5f * sc, cy);
        p.cubicTo(x + 19 * sc, cy - 3 * sc, x + 21 * sc, cy - 3 * sc, x + 22 * sc, cy);
        g.strokePath(p, juce::PathStrokeType(1.5f * sc));
    };
    drawWaveLine(y + 6 * sc);
    drawWaveLine(y + 12 * sc);
    drawWaveLine(y + 18 * sc);
}

void TR8655AudioProcessorEditor::drawIconEqualizer(juce::Graphics& g, float x, float y, float s)
{
    g.setColour(juce::Colour(0xff3B82F6));
    float sc = s / 24.0f;
    juce::Path p;
    p.startNewSubPath(x + 2 * sc, y + 12 * sc);
    p.lineTo(x + 6 * sc, y + 12 * sc);
    p.lineTo(x + 9 * sc, y + 2 * sc);
    p.lineTo(x + 15 * sc, y + 22 * sc);
    p.lineTo(x + 18 * sc, y + 12 * sc);
    p.lineTo(x + 22 * sc, y + 12 * sc);
    g.strokePath(p, juce::PathStrokeType(1.5f * sc));
}

void TR8655AudioProcessorEditor::drawIconMaster(juce::Graphics& g, float x, float y, float s)
{
    g.setColour(juce::Colour(0xff3B82F6));
    float sc = s / 24.0f;

    juce::Path sp;
    sp.startNewSubPath(x + 11 * sc, y + 5 * sc);
    sp.lineTo(x + 6 * sc, y + 8 * sc);
    sp.lineTo(x + 3 * sc, y + 8 * sc);
    sp.lineTo(x + 3 * sc, y + 16 * sc);
    sp.lineTo(x + 6 * sc, y + 16 * sc);
    sp.lineTo(x + 11 * sc, y + 19 * sc);
    sp.closeSubPath();
    g.fillPath(sp);

    juce::Path w1;
    w1.addCentredArc(x + 14 * sc, y + 12 * sc, 3 * sc, 5 * sc,
                      0.0f, -0.6f, 0.6f, true);
    g.strokePath(w1, juce::PathStrokeType(1.5f * sc));

    juce::Path w2;
    w2.addCentredArc(x + 16 * sc, y + 12 * sc, 4 * sc, 8 * sc,
                      0.0f, -0.7f, 0.7f, true);
    g.strokePath(w2, juce::PathStrokeType(1.5f * sc));
}
