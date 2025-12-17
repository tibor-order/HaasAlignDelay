#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"
#include "UI/NuroColors.h"

//==============================================================================
// HaasAlignDelayEditor - Nuro Audio Style UI
//==============================================================================
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

void HaasAlignDelayEditor::drawPanel(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& title)
{
    // Panel background with subtle gradient
    juce::ColourGradient panelGradient(
        UI::NuroColors::colour(UI::NuroColors::panelBackgroundLight),
        bounds.getX(), bounds.getY(),
        UI::NuroColors::colour(UI::NuroColors::panelBackground),
        bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(panelGradient);
    g.fillRoundedRectangle(bounds, 8.0f);

    // Subtle border
    g.setColour(UI::NuroColors::colour(UI::NuroColors::borderDark));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

    // Inner shadow for depth
    g.setColour(juce::Colours::black.withAlpha(0.15f));
    g.drawRoundedRectangle(bounds.reduced(1), 7.0f, 1.0f);

    // Title - centered at top
    if (title.isNotEmpty())
    {
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentCyan));
        g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
        g.drawText(title, bounds.getX(), bounds.getY() + 12,
                   bounds.getWidth(), 16, juce::Justification::centred);
    }
}

void HaasAlignDelayEditor::drawValueDisplay(juce::Graphics& g, juce::Rectangle<float> bounds,
                                             const juce::String& value, const juce::String& subValue)
{
    // Value background
    g.setColour(UI::NuroColors::colour(UI::NuroColors::backgroundBlack));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Main value
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textWhite));
    g.setFont(juce::FontOptions(15.0f).withStyle("Bold"));
    g.drawText(value, bounds, juce::Justification::centred);

    // Sub value
    if (subValue.isNotEmpty())
    {
        auto subBounds = bounds.translated(0, bounds.getHeight() + 4);
        subBounds.setHeight(14);

        bool isCorrection = subValue.startsWith("->");
        g.setColour(isCorrection
            ? UI::NuroColors::colour(UI::NuroColors::accentYellow)
            : UI::NuroColors::colour(UI::NuroColors::textMuted));
        g.setFont(juce::FontOptions(10.0f));
        g.drawText(subValue, subBounds, juce::Justification::centred);
    }
}

void HaasAlignDelayEditor::drawCorrectionIndicator(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    bool correctionActive = processorRef.isCorrectionActive();
    float correctionAmount = processorRef.getCorrectionAmount();

    float indicatorSize = 10.0f;
    float indicatorX = bounds.getRight() - indicatorSize - 12;
    float indicatorY = bounds.getY() + 10;

    if (correctionActive)
    {
        // Pulsing glow effect
        float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(juce::Time::getMillisecondCounter()) * 0.006f);

        // Outer glow layers
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentYellow).withAlpha(0.15f * pulse));
        g.fillEllipse(indicatorX - 8, indicatorY - 8, indicatorSize + 16, indicatorSize + 16);

        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentYellow).withAlpha(0.3f * pulse));
        g.fillEllipse(indicatorX - 4, indicatorY - 4, indicatorSize + 8, indicatorSize + 8);

        // Core LED
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentYellow));
        g.fillEllipse(indicatorX, indicatorY, indicatorSize, indicatorSize);

        // Bright center
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentYellowBright));
        g.fillEllipse(indicatorX + 2, indicatorY + 2, indicatorSize - 4, indicatorSize - 4);

        // Correction amount text
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentYellow));
        g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
        juce::String corrText = "-" + juce::String(static_cast<int>(correctionAmount * 100)) + "%";
        g.drawText(corrText, indicatorX - 40, indicatorY - 1, 35, 12, juce::Justification::centredRight);
    }
    else if (autoPhaseButton.getToggleState())
    {
        // Dim green when auto phase is on but not correcting
        g.setColour(UI::NuroColors::colour(UI::NuroColors::meterGreen).withAlpha(0.15f));
        g.fillEllipse(indicatorX - 4, indicatorY - 4, indicatorSize + 8, indicatorSize + 8);

        g.setColour(UI::NuroColors::colour(UI::NuroColors::meterGreen).withAlpha(0.5f));
        g.fillEllipse(indicatorX, indicatorY, indicatorSize, indicatorSize);
    }
}

void HaasAlignDelayEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // === BACKGROUND: Deep black with subtle gradient ===
    juce::ColourGradient bgGradient(
        UI::NuroColors::colour(UI::NuroColors::backgroundDark), 0, 0,
        UI::NuroColors::colour(UI::NuroColors::backgroundBlack), 0, bounds.getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillRect(bounds);

    // === HEADER BAR (60px) ===
    float headerHeight = 60.0f;
    auto headerBounds = juce::Rectangle<float>(0, 0, bounds.getWidth(), headerHeight);

    g.setColour(UI::NuroColors::colour(UI::NuroColors::backgroundBlack));
    g.fillRect(headerBounds);

    // Header bottom line
    g.setColour(UI::NuroColors::colour(UI::NuroColors::borderMedium).withAlpha(0.5f));
    g.fillRect(0.0f, headerHeight - 1, bounds.getWidth(), 1.0f);

    // Logo / Plugin Name
    if (logoImage.isValid())
    {
        float logoHeight = 36.0f;
        float logoWidth = logoImage.getWidth() * (logoHeight / logoImage.getHeight());
        g.drawImage(logoImage, 30, 12, static_cast<int>(logoWidth), static_cast<int>(logoHeight),
                    0, 0, logoImage.getWidth(), logoImage.getHeight());
    }
    else
    {
        // Fallback text logo
        g.setFont(juce::FontOptions(28.0f).withStyle("Bold"));
        g.setColour(UI::NuroColors::colour(UI::NuroColors::textWhite));
        g.drawText("HAAS", 30, 15, 80, 32, juce::Justification::centredLeft);

        // "PRO" badge
        auto proBadge = juce::Rectangle<float>(110, 20, 45, 22);
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentCyan));
        g.fillRoundedRectangle(proBadge, 4.0f);
        g.setColour(UI::NuroColors::colour(UI::NuroColors::backgroundBlack));
        g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
        g.drawText("PRO", proBadge, juce::Justification::centred);
    }

    // === MAIN CONTENT AREA ===
    float footerHeight = 80.0f;
    float sidePadding = 30.0f;
    float topPadding = 30.0f;
    float panelGap = 20.0f;

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
    float knobSize = 100.0f;
    float valueDisplayWidth = 80.0f;
    float valueDisplayHeight = 24.0f;

    auto drawKnobValue = [&](juce::Rectangle<float> panel, double value, const juce::String& unit,
                             const juce::String& subValue = {})
    {
        float centerX = panel.getCentreX();
        float valueY = panel.getY() + 36 + knobSize + 20;

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
    float controlCenterX = panel4.getCentreX();

    // Phase Invert label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textMuted));
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("PHASE INVERT", panel4.getX(), panel4.getY() + 36,
               panel4.getWidth(), 14, juce::Justification::centred);

    // Mix label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::accentCyan));
    g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    g.drawText("MIX", panel4.getX(), panel4.getY() + 110,
               panel4.getWidth(), 14, juce::Justification::centred);

    // Mix value display
    float mixKnobSize = 70.0f;
    auto mixValueBounds = juce::Rectangle<float>(
        controlCenterX - 40, panel4.getY() + 130 + mixKnobSize + 12, 80, 22);
    g.setColour(UI::NuroColors::colour(UI::NuroColors::backgroundBlack));
    g.fillRoundedRectangle(mixValueBounds, 4.0f);
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textWhite));
    g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.drawText(juce::String(static_cast<int>(mixSlider.getValue())) + "%",
               mixValueBounds, juce::Justification::centred);

    // Auto Phase label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textMuted));
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("AUTO PHASE", panel4.getX(), panel4.getY() + panelHeight - 95,
               panel4.getWidth(), 14, juce::Justification::centred);

    // Safety label
    g.drawText("SAFETY", panel4.getX(), panel4.getY() + panelHeight - 48,
               panel4.getWidth(), 14, juce::Justification::centred);

    // === FOOTER BAR (80px) ===
    auto footerBounds = juce::Rectangle<float>(0, bounds.getHeight() - footerHeight, bounds.getWidth(), footerHeight);

    g.setColour(UI::NuroColors::colour(UI::NuroColors::backgroundBlack));
    g.fillRect(footerBounds);

    // Footer top line
    g.setColour(UI::NuroColors::colour(UI::NuroColors::borderMedium).withAlpha(0.5f));
    g.fillRect(0.0f, footerBounds.getY(), bounds.getWidth(), 1.0f);

    float meterY = footerBounds.getY() + 25;

    // Input label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textMuted));
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("INPUT", 30, static_cast<int>(meterY - 3), 50, 16, juce::Justification::centredLeft);

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

    float corrWidth = 120.0f;
    float corrX = bounds.getCentreX() - corrWidth / 2.0f;

    // Phase label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textMuted));
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("PHASE", corrX, meterY - 18, corrWidth, 14, juce::Justification::centred);

    // Correlation value
    g.setColour(corrColor);
    g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
    juce::String corrValueStr = (corr >= 0 ? "+" : "") + juce::String(corr, 2);
    g.drawText(corrValueStr, corrX, meterY + 22, corrWidth, 16, juce::Justification::centred);

    // Scale markers
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textDisabled));
    g.setFont(juce::FontOptions(9.0f));
    g.drawText("-1", corrX - 15, meterY + 22, 24, 14, juce::Justification::centred);
    g.drawText("+1", corrX + corrWidth - 9, meterY + 22, 24, 14, juce::Justification::centred);

    // Output label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textMuted));
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("OUTPUT", static_cast<int>(bounds.getWidth() - 80), static_cast<int>(meterY - 3),
               50, 16, juce::Justification::centredRight);
}

void HaasAlignDelayEditor::resized()
{
    auto bounds = getLocalBounds();

    // Layout constants
    float headerHeight = 60.0f;
    float footerHeight = 80.0f;
    float sidePadding = 30.0f;
    float topPadding = 30.0f;
    float panelGap = 20.0f;

    // Resize triangle in bottom right corner (16x16, 8px from edges)
    resizeTriangle.setBounds(bounds.getWidth() - 24, bounds.getHeight() - 24, 16, 16);

    // Bypass button - top right
    bypassButton.setBounds(bounds.getWidth() - 110, 17, 80, 28);

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

    // Center the panels
    float totalPanelsWidth = mainPanelWidth * 3 + controlPanelWidth + totalGaps;
    float startX = contentArea.getX() + (contentArea.getWidth() - totalPanelsWidth) / 2.0f;

    // Knob positioning
    float knobSize = 100.0f;
    float knobY = contentArea.getY() + 36;

    // Panel centers
    float panel1CenterX = startX + mainPanelWidth / 2.0f;
    float panel2CenterX = startX + mainPanelWidth + panelGap + mainPanelWidth / 2.0f;
    float panel3CenterX = startX + (mainPanelWidth + panelGap) * 2 + mainPanelWidth / 2.0f;
    float panel4X = startX + (mainPanelWidth + panelGap) * 3;
    float panel4CenterX = panel4X + controlPanelWidth / 2.0f;

    // Position main knobs (centered in panels)
    delayLeftSlider.setBounds(static_cast<int>(panel1CenterX - knobSize / 2), static_cast<int>(knobY),
                              static_cast<int>(knobSize), static_cast<int>(knobSize));
    widthSlider.setBounds(static_cast<int>(panel2CenterX - knobSize / 2), static_cast<int>(knobY),
                          static_cast<int>(knobSize), static_cast<int>(knobSize));
    delayRightSlider.setBounds(static_cast<int>(panel3CenterX - knobSize / 2), static_cast<int>(knobY),
                               static_cast<int>(knobSize), static_cast<int>(knobSize));

    // Controls panel elements
    float panelHeight = contentArea.getHeight();

    // Phase buttons (L and R side by side)
    float phaseButtonWidth = 42.0f;
    float phaseButtonHeight = 32.0f;
    float phaseButtonSpacing = 10.0f;
    float phaseButtonsY = contentArea.getY() + 54;

    phaseLeftButton.setBounds(static_cast<int>(panel4CenterX - phaseButtonWidth - phaseButtonSpacing / 2),
                              static_cast<int>(phaseButtonsY),
                              static_cast<int>(phaseButtonWidth), static_cast<int>(phaseButtonHeight));
    phaseRightButton.setBounds(static_cast<int>(panel4CenterX + phaseButtonSpacing / 2),
                               static_cast<int>(phaseButtonsY),
                               static_cast<int>(phaseButtonWidth), static_cast<int>(phaseButtonHeight));

    // Mix knob
    float mixKnobSize = 70.0f;
    float mixKnobY = contentArea.getY() + 130;
    mixSlider.setBounds(static_cast<int>(panel4CenterX - mixKnobSize / 2), static_cast<int>(mixKnobY),
                        static_cast<int>(mixKnobSize), static_cast<int>(mixKnobSize));

    // Auto Phase button
    float autoButtonY = contentArea.getY() + panelHeight - 78;
    autoPhaseButton.setBounds(static_cast<int>(panel4CenterX - 40), static_cast<int>(autoButtonY),
                              80, 28);

    // Phase Safety selector
    float safetyY = contentArea.getY() + panelHeight - 32;
    phaseSafetySelector.setBounds(static_cast<int>(panel4CenterX - 50), static_cast<int>(safetyY),
                                  100, 24);

    // Meters in footer
    float meterHeight = 16.0f;
    float meterY = bounds.getHeight() - footerHeight + 25;

    inputMeter.setBounds(85, static_cast<int>(meterY), 180, static_cast<int>(meterHeight));

    float corrWidth = 120.0f;
    float corrX = bounds.getWidth() / 2.0f - corrWidth / 2.0f;
    correlationMeter.setBounds(static_cast<int>(corrX), static_cast<int>(meterY),
                               static_cast<int>(corrWidth), static_cast<int>(meterHeight));

    outputMeter.setBounds(bounds.getWidth() - 265, static_cast<int>(meterY), 180, static_cast<int>(meterHeight));
}

void HaasAlignDelayEditor::timerCallback()
{
    // Update meters from processor
    inputMeter.setLevel(processorRef.getInputLevel());
    outputMeter.setLevel(processorRef.getOutputLevel());
    correlationMeter.setCorrelation(processorRef.getCorrelation());

    repaint();
}
