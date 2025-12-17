#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"
#include "UI/NuroColors.h"

//==============================================================================
// HaasAlignDelayEditor - Nuro Audio Style UI with Proportional Scaling
//==============================================================================

// Base dimensions (Medium size - everything is designed at this size)
static constexpr float BASE_WIDTH = 1000.0f;
static constexpr float BASE_HEIGHT = 625.0f;

HaasAlignDelayEditor::HaasAlignDelayEditor(HaasAlignDelayProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&nuroLookAndFeel);

    logoImage = juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);

    auto setupSlider = [this](juce::Slider& slider)
    {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        addAndMakeVisible(slider);
    };

    setupSlider(delayLeftSlider);
    setupSlider(delayRightSlider);
    setupSlider(widthSlider);
    setupSlider(mixSlider);

    auto setupButton = [this](juce::TextButton& button, const juce::String& text)
    {
        button.setButtonText(text);
        button.setClickingTogglesState(true);
        addAndMakeVisible(button);
    };

    setupButton(phaseLeftButton, "L");
    setupButton(phaseRightButton, "R");
    setupButton(bypassButton, "BYPASS");
    setupButton(autoPhaseButton, "AUTO");

    // Phase Safety selector
    phaseSafetySelector.addItem("Relaxed", 1);
    phaseSafetySelector.addItem("Balanced", 2);
    phaseSafetySelector.addItem("Strict", 3);
    phaseSafetySelector.setSelectedId(2, juce::dontSendNotification);
    addAndMakeVisible(phaseSafetySelector);

    // Setup resize triangle with size presets
    std::vector<UI::ResizeTriangle::SizePreset> presets;
    for (int i = 0; i < numSizePresets; ++i)
    {
        presets.push_back({ sizePresets[i].name, sizePresets[i].width, sizePresets[i].height });
    }
    resizeTriangle.setSizePresets(presets);
    resizeTriangle.setCurrentPresetIndex(currentSizePreset);
    resizeTriangle.onSizeSelected = [this](int index)
    {
        setScalePreset(index);
    };
    addAndMakeVisible(resizeTriangle);

    // Add meter components
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    addAndMakeVisible(correlationMeter);

    auto& apvts = processorRef.getAPVTS();

    delayLeftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "delayLeft", delayLeftSlider);
    delayRightAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "delayRight", delayRightSlider);
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "width", widthSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "mix", mixSlider);
    phaseLeftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "phaseLeft", phaseLeftButton);
    phaseRightAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "phaseRight", phaseRightButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "bypass", bypassButton);
    autoPhaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "autoPhase", autoPhaseButton);
    phaseSafetyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "phaseSafety", phaseSafetySelector);

    delayLeftSlider.onValueChange = [this]() { repaint(); };
    delayRightSlider.onValueChange = [this]() { repaint(); };
    widthSlider.onValueChange = [this]() { repaint(); };
    mixSlider.onValueChange = [this]() { repaint(); };

    startTimerHz(30);

    // Start at Medium preset (1000x625)
    currentSizePreset = 1;
    setSize(sizePresets[currentSizePreset].width, sizePresets[currentSizePreset].height);
    setResizable(false, false);
}

HaasAlignDelayEditor::~HaasAlignDelayEditor()
{
    setLookAndFeel(nullptr);
    stopTimer();
}

void HaasAlignDelayEditor::setScalePreset(int presetIndex)
{
    if (presetIndex < 0 || presetIndex >= numSizePresets)
        return;

    currentSizePreset = presetIndex;
    resizeTriangle.setCurrentPresetIndex(presetIndex);

    int newWidth = sizePresets[presetIndex].width;
    int newHeight = sizePresets[presetIndex].height;

    setSize(newWidth, newHeight);
}

// Helper to get current scale factor
float HaasAlignDelayEditor::getScaleFactor() const
{
    return static_cast<float>(getWidth()) / BASE_WIDTH;
}

void HaasAlignDelayEditor::drawPanel(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& title)
{
    float s = getScaleFactor();

    // Panel background with subtle gradient
    juce::ColourGradient panelGradient(
        UI::NuroColors::colour(UI::NuroColors::panelBackgroundLight),
        bounds.getX(), bounds.getY(),
        UI::NuroColors::colour(UI::NuroColors::panelBackground),
        bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(panelGradient);
    g.fillRoundedRectangle(bounds, 8.0f * s);

    // Subtle border
    g.setColour(UI::NuroColors::colour(UI::NuroColors::borderDark));
    g.drawRoundedRectangle(bounds, 8.0f * s, 1.0f * s);

    // Inner shadow for depth
    g.setColour(juce::Colours::black.withAlpha(0.15f));
    g.drawRoundedRectangle(bounds.reduced(1 * s), 7.0f * s, 1.0f * s);

    // Title - centered at top
    if (title.isNotEmpty())
    {
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentCyan));
        g.setFont(juce::FontOptions(12.0f * s).withStyle("Bold"));
        g.drawText(title, bounds.getX(), bounds.getY() + 12 * s,
                   bounds.getWidth(), 16 * s, juce::Justification::centred);
    }
}

void HaasAlignDelayEditor::drawValueDisplay(juce::Graphics& g, juce::Rectangle<float> bounds,
                                             const juce::String& value, const juce::String& subValue)
{
    float s = getScaleFactor();

    // Value background
    g.setColour(UI::NuroColors::colour(UI::NuroColors::backgroundBlack));
    g.fillRoundedRectangle(bounds, 4.0f * s);

    // Main value
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textWhite));
    g.setFont(juce::FontOptions(15.0f * s).withStyle("Bold"));
    g.drawText(value, bounds, juce::Justification::centred);

    // Sub value
    if (subValue.isNotEmpty())
    {
        auto subBounds = bounds.translated(0, bounds.getHeight() + 4 * s);
        subBounds.setHeight(14 * s);

        bool isCorrection = subValue.startsWith("->");
        g.setColour(isCorrection
            ? UI::NuroColors::colour(UI::NuroColors::accentYellow)
            : UI::NuroColors::colour(UI::NuroColors::textMuted));
        g.setFont(juce::FontOptions(10.0f * s));
        g.drawText(subValue, subBounds, juce::Justification::centred);
    }
}

void HaasAlignDelayEditor::drawCorrectionIndicator(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    float s = getScaleFactor();
    bool correctionActive = processorRef.isCorrectionActive();
    float correctionAmount = processorRef.getCorrectionAmount();

    float indicatorSize = 10.0f * s;
    float indicatorX = bounds.getRight() - indicatorSize - 12 * s;
    float indicatorY = bounds.getY() + 10 * s;

    if (correctionActive)
    {
        // Pulsing glow effect
        float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(juce::Time::getMillisecondCounter()) * 0.006f);

        // Outer glow layers
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentYellow).withAlpha(0.15f * pulse));
        g.fillEllipse(indicatorX - 8 * s, indicatorY - 8 * s, indicatorSize + 16 * s, indicatorSize + 16 * s);

        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentYellow).withAlpha(0.3f * pulse));
        g.fillEllipse(indicatorX - 4 * s, indicatorY - 4 * s, indicatorSize + 8 * s, indicatorSize + 8 * s);

        // Core LED
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentYellow));
        g.fillEllipse(indicatorX, indicatorY, indicatorSize, indicatorSize);

        // Bright center
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentYellowBright));
        g.fillEllipse(indicatorX + 2 * s, indicatorY + 2 * s, indicatorSize - 4 * s, indicatorSize - 4 * s);

        // Correction amount text
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentYellow));
        g.setFont(juce::FontOptions(10.0f * s).withStyle("Bold"));
        juce::String corrText = "-" + juce::String(static_cast<int>(correctionAmount * 100)) + "%";
        g.drawText(corrText, indicatorX - 40 * s, indicatorY - 1 * s, 35 * s, 12 * s, juce::Justification::centredRight);
    }
    else if (autoPhaseButton.getToggleState())
    {
        // Dim green when auto phase is on but not correcting
        g.setColour(UI::NuroColors::colour(UI::NuroColors::meterGreen).withAlpha(0.15f));
        g.fillEllipse(indicatorX - 4 * s, indicatorY - 4 * s, indicatorSize + 8 * s, indicatorSize + 8 * s);

        g.setColour(UI::NuroColors::colour(UI::NuroColors::meterGreen).withAlpha(0.5f));
        g.fillEllipse(indicatorX, indicatorY, indicatorSize, indicatorSize);
    }
}

void HaasAlignDelayEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float s = getScaleFactor();

    // === BACKGROUND: Deep black with subtle gradient ===
    juce::ColourGradient bgGradient(
        UI::NuroColors::colour(UI::NuroColors::backgroundDark), 0, 0,
        UI::NuroColors::colour(UI::NuroColors::backgroundBlack), 0, bounds.getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillRect(bounds);

    // === HEADER BAR (60px at base) ===
    float headerHeight = 60.0f * s;
    auto headerBounds = juce::Rectangle<float>(0, 0, bounds.getWidth(), headerHeight);

    g.setColour(UI::NuroColors::colour(UI::NuroColors::backgroundBlack));
    g.fillRect(headerBounds);

    // Header bottom line
    g.setColour(UI::NuroColors::colour(UI::NuroColors::borderMedium).withAlpha(0.5f));
    g.fillRect(0.0f, headerHeight - 1 * s, bounds.getWidth(), 1.0f * s);

    // Logo / Plugin Name
    if (logoImage.isValid())
    {
        float logoHeight = 36.0f * s;
        float logoWidth = logoImage.getWidth() * (logoHeight / logoImage.getHeight());
        g.drawImage(logoImage, static_cast<int>(30 * s), static_cast<int>(12 * s),
                    static_cast<int>(logoWidth), static_cast<int>(logoHeight),
                    0, 0, logoImage.getWidth(), logoImage.getHeight());
    }
    else
    {
        // Fallback text logo
        g.setFont(juce::FontOptions(28.0f * s).withStyle("Bold"));
        g.setColour(UI::NuroColors::colour(UI::NuroColors::textWhite));
        g.drawText("HAAS", static_cast<int>(30 * s), static_cast<int>(15 * s),
                   static_cast<int>(80 * s), static_cast<int>(32 * s), juce::Justification::centredLeft);

        // "PRO" badge
        auto proBadge = juce::Rectangle<float>(110 * s, 20 * s, 45 * s, 22 * s);
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentCyan));
        g.fillRoundedRectangle(proBadge, 4.0f * s);
        g.setColour(UI::NuroColors::colour(UI::NuroColors::backgroundBlack));
        g.setFont(juce::FontOptions(12.0f * s).withStyle("Bold"));
        g.drawText("PRO", proBadge, juce::Justification::centred);
    }

    // === MAIN CONTENT AREA ===
    float footerHeight = 80.0f * s;
    float sidePadding = 30.0f * s;
    float topPadding = 30.0f * s;
    float panelGap = 20.0f * s;

    auto contentArea = bounds;
    contentArea.removeFromTop(headerHeight);
    contentArea.removeFromBottom(footerHeight);
    contentArea = contentArea.reduced(sidePadding, topPadding);

    // Calculate panel sizes - equal width for main 3 panels, slightly wider for controls
    float totalGaps = panelGap * 3;
    float availableWidth = contentArea.getWidth() - totalGaps;
    float mainPanelWidth = availableWidth * 0.22f;
    float controlPanelWidth = availableWidth * 0.34f;
    float panelHeight = contentArea.getHeight();

    // Center the panels horizontally
    float totalPanelsWidth = mainPanelWidth * 3 + controlPanelWidth + totalGaps;
    float startX = contentArea.getX() + (contentArea.getWidth() - totalPanelsWidth) / 2.0f;

    // Panel 1: LEFT DELAY
    auto panel1 = juce::Rectangle<float>(startX, contentArea.getY(), mainPanelWidth, panelHeight);
    drawPanel(g, panel1, "LEFT DELAY");

    // Panel 2: WIDTH
    auto panel2 = juce::Rectangle<float>(panel1.getRight() + panelGap, contentArea.getY(), mainPanelWidth, panelHeight);
    drawPanel(g, panel2, "WIDTH");
    drawCorrectionIndicator(g, panel2);

    // Panel 3: RIGHT DELAY
    auto panel3 = juce::Rectangle<float>(panel2.getRight() + panelGap, contentArea.getY(), mainPanelWidth, panelHeight);
    drawPanel(g, panel3, "RIGHT DELAY");

    // Panel 4: CONTROLS
    auto panel4 = juce::Rectangle<float>(panel3.getRight() + panelGap, contentArea.getY(), controlPanelWidth, panelHeight);
    drawPanel(g, panel4, "CONTROLS");

    // === DRAW VALUE DISPLAYS ===
    float knobSize = 100.0f * s;
    float valueDisplayWidth = 80.0f * s;
    float valueDisplayHeight = 24.0f * s;

    auto drawKnobValue = [&](juce::Rectangle<float> panel, double value, const juce::String& unit,
                             const juce::String& subValue = {})
    {
        float centerX = panel.getCentreX();
        float valueY = panel.getY() + 36 * s + knobSize + 20 * s;

        juce::String valueStr;
        if (unit == "ms")
            valueStr = juce::String(value, 1) + " " + unit;
        else
            valueStr = juce::String(static_cast<int>(value)) + unit;

        auto valueBounds = juce::Rectangle<float>(
            centerX - valueDisplayWidth / 2, valueY, valueDisplayWidth, valueDisplayHeight);
        drawValueDisplay(g, valueBounds, valueStr, subValue);
    };

    // Left Delay value
    juce::String leftSubValue = juce::String(static_cast<int>(processorRef.msToSamples(
        static_cast<float>(delayLeftSlider.getValue())))) + " smp";
    drawKnobValue(panel1, delayLeftSlider.getValue(), "ms", leftSubValue);

    // Width value with correction indicator
    juce::String widthSubValue;
    if (processorRef.isCorrectionActive())
        widthSubValue = "-> " + juce::String(static_cast<int>(processorRef.getEffectiveWidth())) + "%";
    drawKnobValue(panel2, widthSlider.getValue(), "%", widthSubValue);

    // Right Delay value
    juce::String rightSubValue = juce::String(static_cast<int>(processorRef.msToSamples(
        static_cast<float>(delayRightSlider.getValue())))) + " smp";
    drawKnobValue(panel3, delayRightSlider.getValue(), "ms", rightSubValue);

    // === CONTROLS PANEL LABELS ===
    // Phase Invert label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textMuted));
    g.setFont(juce::FontOptions(10.0f * s));
    g.drawText("PHASE INVERT", panel4.getX(), panel4.getY() + 36 * s,
               panel4.getWidth(), 14 * s, juce::Justification::centred);

    // Mix label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::accentCyan));
    g.setFont(juce::FontOptions(11.0f * s).withStyle("Bold"));
    g.drawText("MIX", panel4.getX(), panel4.getY() + 110 * s,
               panel4.getWidth(), 14 * s, juce::Justification::centred);

    // Mix value display
    float mixKnobSize = 70.0f * s;
    float mixValueY = panel4.getY() + 130 * s + mixKnobSize + 12 * s;
    auto mixValueBounds = juce::Rectangle<float>(
        panel4.getCentreX() - 40 * s, mixValueY, 80 * s, 22 * s);
    g.setColour(UI::NuroColors::colour(UI::NuroColors::backgroundBlack));
    g.fillRoundedRectangle(mixValueBounds, 4.0f * s);
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textWhite));
    g.setFont(juce::FontOptions(14.0f * s).withStyle("Bold"));
    g.drawText(juce::String(static_cast<int>(mixSlider.getValue())) + "%",
               mixValueBounds, juce::Justification::centred);

    // Auto Phase label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textMuted));
    g.setFont(juce::FontOptions(10.0f * s));
    g.drawText("AUTO PHASE", panel4.getX(), panel4.getY() + panelHeight - 95 * s,
               panel4.getWidth(), 14 * s, juce::Justification::centred);

    // Safety label
    g.drawText("SAFETY", panel4.getX(), panel4.getY() + panelHeight - 48 * s,
               panel4.getWidth(), 14 * s, juce::Justification::centred);

    // === FOOTER BAR (80px at base) ===
    auto footerBounds = juce::Rectangle<float>(0, bounds.getHeight() - footerHeight, bounds.getWidth(), footerHeight);

    g.setColour(UI::NuroColors::colour(UI::NuroColors::backgroundBlack));
    g.fillRect(footerBounds);

    // Footer top line
    g.setColour(UI::NuroColors::colour(UI::NuroColors::borderMedium).withAlpha(0.5f));
    g.fillRect(0.0f, footerBounds.getY(), bounds.getWidth(), 1.0f * s);

    float meterY = footerBounds.getY() + 25 * s;

    // Input label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textMuted));
    g.setFont(juce::FontOptions(11.0f * s));
    g.drawText("INPUT", static_cast<int>(30 * s), static_cast<int>(meterY - 3 * s),
               static_cast<int>(50 * s), static_cast<int>(16 * s), juce::Justification::centredLeft);

    // Correlation section (centered)
    float corr = processorRef.getCorrelation();
    juce::Colour corrColor;
    if (corr > 0.5f)
        corrColor = UI::NuroColors::colour(UI::NuroColors::meterGreen);
    else if (corr > 0.3f)
        corrColor = UI::NuroColors::colour(UI::NuroColors::meterYellow);
    else if (corr > 0.0f)
        corrColor = UI::NuroColors::colour(UI::NuroColors::meterOrange);
    else
        corrColor = UI::NuroColors::colour(UI::NuroColors::meterRed);

    float corrWidth = 120.0f * s;
    float corrX = bounds.getCentreX() - corrWidth / 2.0f;

    // Phase label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textMuted));
    g.setFont(juce::FontOptions(10.0f * s));
    g.drawText("PHASE", corrX, meterY - 18 * s, corrWidth, 14 * s, juce::Justification::centred);

    // Correlation value
    g.setColour(corrColor);
    g.setFont(juce::FontOptions(12.0f * s).withStyle("Bold"));
    juce::String corrValueStr = (corr >= 0 ? "+" : "") + juce::String(corr, 2);
    g.drawText(corrValueStr, corrX, meterY + 22 * s, corrWidth, 16 * s, juce::Justification::centred);

    // Scale markers
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textDisabled));
    g.setFont(juce::FontOptions(9.0f * s));
    g.drawText("-1", corrX - 15 * s, meterY + 22 * s, 24 * s, 14 * s, juce::Justification::centred);
    g.drawText("+1", corrX + corrWidth - 9 * s, meterY + 22 * s, 24 * s, 14 * s, juce::Justification::centred);

    // Output label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textMuted));
    g.setFont(juce::FontOptions(11.0f * s));
    g.drawText("OUTPUT", static_cast<int>(bounds.getWidth() - 80 * s), static_cast<int>(meterY - 3 * s),
               static_cast<int>(50 * s), static_cast<int>(16 * s), juce::Justification::centredRight);
}

void HaasAlignDelayEditor::resized()
{
    auto bounds = getLocalBounds();
    float s = getScaleFactor();

    // Layout constants (scaled)
    float headerHeight = 60.0f * s;
    float footerHeight = 80.0f * s;
    float sidePadding = 30.0f * s;
    float topPadding = 30.0f * s;
    float panelGap = 20.0f * s;

    // Resize triangle in bottom right corner (fixed 16x16, 8px from edges)
    resizeTriangle.setBounds(bounds.getWidth() - 24, bounds.getHeight() - 24, 16, 16);

    // Bypass button - top right (scaled)
    bypassButton.setBounds(static_cast<int>(bounds.getWidth() - 110 * s),
                           static_cast<int>(17 * s),
                           static_cast<int>(80 * s),
                           static_cast<int>(28 * s));

    // Main content area
    auto contentArea = bounds.toFloat();
    contentArea.removeFromTop(headerHeight);
    contentArea.removeFromBottom(footerHeight);
    contentArea = contentArea.reduced(sidePadding, topPadding);

    // Calculate panel sizes
    float totalGaps = panelGap * 3;
    float availableWidth = contentArea.getWidth() - totalGaps;
    float mainPanelWidth = availableWidth * 0.22f;
    float controlPanelWidth = availableWidth * 0.34f;
    float panelHeight = contentArea.getHeight();

    // Center the panels
    float totalPanelsWidth = mainPanelWidth * 3 + controlPanelWidth + totalGaps;
    float startX = contentArea.getX() + (contentArea.getWidth() - totalPanelsWidth) / 2.0f;

    // Knob positioning (scaled)
    float knobSize = 100.0f * s;
    float knobY = contentArea.getY() + 36 * s;

    // Panel centers
    float panel1CenterX = startX + mainPanelWidth / 2.0f;
    float panel2CenterX = startX + mainPanelWidth + panelGap + mainPanelWidth / 2.0f;
    float panel3CenterX = startX + (mainPanelWidth + panelGap) * 2 + mainPanelWidth / 2.0f;
    float panel4X = startX + (mainPanelWidth + panelGap) * 3;
    float panel4CenterX = panel4X + controlPanelWidth / 2.0f;

    // Position main knobs (centered in panels)
    delayLeftSlider.setBounds(static_cast<int>(panel1CenterX - knobSize / 2),
                              static_cast<int>(knobY),
                              static_cast<int>(knobSize),
                              static_cast<int>(knobSize));
    widthSlider.setBounds(static_cast<int>(panel2CenterX - knobSize / 2),
                          static_cast<int>(knobY),
                          static_cast<int>(knobSize),
                          static_cast<int>(knobSize));
    delayRightSlider.setBounds(static_cast<int>(panel3CenterX - knobSize / 2),
                               static_cast<int>(knobY),
                               static_cast<int>(knobSize),
                               static_cast<int>(knobSize));

    // Controls panel elements (scaled)
    // Phase buttons (L and R side by side)
    float phaseButtonWidth = 42.0f * s;
    float phaseButtonHeight = 32.0f * s;
    float phaseButtonSpacing = 10.0f * s;
    float phaseButtonsY = contentArea.getY() + 54 * s;

    phaseLeftButton.setBounds(static_cast<int>(panel4CenterX - phaseButtonWidth - phaseButtonSpacing / 2),
                              static_cast<int>(phaseButtonsY),
                              static_cast<int>(phaseButtonWidth),
                              static_cast<int>(phaseButtonHeight));
    phaseRightButton.setBounds(static_cast<int>(panel4CenterX + phaseButtonSpacing / 2),
                               static_cast<int>(phaseButtonsY),
                               static_cast<int>(phaseButtonWidth),
                               static_cast<int>(phaseButtonHeight));

    // Mix knob (scaled)
    float mixKnobSize = 70.0f * s;
    float mixKnobY = contentArea.getY() + 130 * s;
    mixSlider.setBounds(static_cast<int>(panel4CenterX - mixKnobSize / 2),
                        static_cast<int>(mixKnobY),
                        static_cast<int>(mixKnobSize),
                        static_cast<int>(mixKnobSize));

    // Auto Phase button (scaled)
    float autoButtonY = contentArea.getY() + panelHeight - 78 * s;
    autoPhaseButton.setBounds(static_cast<int>(panel4CenterX - 40 * s),
                              static_cast<int>(autoButtonY),
                              static_cast<int>(80 * s),
                              static_cast<int>(28 * s));

    // Phase Safety selector (scaled)
    float safetyY = contentArea.getY() + panelHeight - 32 * s;
    phaseSafetySelector.setBounds(static_cast<int>(panel4CenterX - 50 * s),
                                  static_cast<int>(safetyY),
                                  static_cast<int>(100 * s),
                                  static_cast<int>(24 * s));

    // Meters in footer (scaled)
    float meterHeight = 16.0f * s;
    float meterY = bounds.getHeight() - footerHeight + 25 * s;

    inputMeter.setBounds(static_cast<int>(85 * s),
                         static_cast<int>(meterY),
                         static_cast<int>(180 * s),
                         static_cast<int>(meterHeight));

    float corrWidth = 120.0f * s;
    float corrX = bounds.getWidth() / 2.0f - corrWidth / 2.0f;
    correlationMeter.setBounds(static_cast<int>(corrX),
                               static_cast<int>(meterY),
                               static_cast<int>(corrWidth),
                               static_cast<int>(meterHeight));

    outputMeter.setBounds(static_cast<int>(bounds.getWidth() - 265 * s),
                          static_cast<int>(meterY),
                          static_cast<int>(180 * s),
                          static_cast<int>(meterHeight));
}

void HaasAlignDelayEditor::timerCallback()
{
    // Update meters from processor
    inputMeter.setLevel(processorRef.getInputLevel());
    outputMeter.setLevel(processorRef.getOutputLevel());
    correlationMeter.setCorrelation(processorRef.getCorrelation());

    repaint();
}
