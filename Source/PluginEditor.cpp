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

    // Scale picker button
    scaleButton.setButtonText("100%");
    scaleButton.setClickingTogglesState(false);
    scaleButton.onClick = [this]()
    {
        currentScalePreset = (currentScalePreset + 1) % numScalePresets;
        setScalePreset(currentScalePreset);
    };
    addAndMakeVisible(scaleButton);

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

    currentScalePreset = 2;
    setSize(baseWidth, baseHeight);
    setResizable(false, false);
}

HaasAlignDelayEditor::~HaasAlignDelayEditor()
{
    setLookAndFeel(nullptr);
    stopTimer();
}

void HaasAlignDelayEditor::setScalePreset(int presetIndex)
{
    float scaleFactor = scalePresets[presetIndex];
    int newWidth = static_cast<int>(baseWidth * scaleFactor);
    int newHeight = static_cast<int>(baseHeight * scaleFactor);

    int percent = static_cast<int>(scaleFactor * 100);
    scaleButton.setButtonText(juce::String(percent) + "%");

    setSize(newWidth, newHeight);
}

void HaasAlignDelayEditor::drawPanel(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& title)
{
    float scale = getWidth() / static_cast<float>(baseWidth);

    // Panel background with subtle gradient
    juce::ColourGradient panelGradient(
        UI::NuroColors::colour(UI::NuroColors::panelBackgroundLight),
        bounds.getX(), bounds.getY(),
        UI::NuroColors::colour(UI::NuroColors::panelBackground),
        bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(panelGradient);
    g.fillRoundedRectangle(bounds, 8.0f * scale);

    // Subtle border
    g.setColour(UI::NuroColors::colour(UI::NuroColors::borderDark));
    g.drawRoundedRectangle(bounds, 8.0f * scale, 1.0f);

    // Inner shadow for depth
    g.setColour(juce::Colours::black.withAlpha(0.15f));
    g.drawRoundedRectangle(bounds.reduced(1), 7.0f * scale, 1.0f);

    // Title
    if (title.isNotEmpty())
    {
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentCyan));
        g.setFont(juce::FontOptions(11.0f * scale).withStyle("Bold"));
        g.drawText(title, bounds.getX() + 12 * scale, bounds.getY() + 8 * scale,
                   bounds.getWidth() - 24 * scale, 14 * scale, juce::Justification::centredLeft);
    }
}

void HaasAlignDelayEditor::drawValueDisplay(juce::Graphics& g, juce::Rectangle<float> bounds,
                                             const juce::String& value, const juce::String& subValue)
{
    float scale = getWidth() / static_cast<float>(baseWidth);

    // Value background
    g.setColour(UI::NuroColors::colour(UI::NuroColors::backgroundBlack));
    g.fillRoundedRectangle(bounds, 4.0f * scale);

    // Main value
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textWhite));
    g.setFont(juce::FontOptions(14.0f * scale).withStyle("Bold"));
    g.drawText(value, bounds, juce::Justification::centred);

    // Sub value
    if (subValue.isNotEmpty())
    {
        auto subBounds = bounds.translated(0, bounds.getHeight() + 2 * scale);
        subBounds.setHeight(12 * scale);

        bool isCorrection = subValue.startsWith("->");
        g.setColour(isCorrection
            ? UI::NuroColors::colour(UI::NuroColors::accentYellow)
            : UI::NuroColors::colour(UI::NuroColors::textMuted));
        g.setFont(juce::FontOptions(9.0f * scale));
        g.drawText(subValue, subBounds, juce::Justification::centred);
    }
}

void HaasAlignDelayEditor::drawCorrectionIndicator(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    float scale = getWidth() / static_cast<float>(baseWidth);
    bool correctionActive = processorRef.isCorrectionActive();
    float correctionAmount = processorRef.getCorrectionAmount();

    float indicatorSize = 10.0f * scale;
    float indicatorX = bounds.getRight() - indicatorSize - 10 * scale;
    float indicatorY = bounds.getY() + 8 * scale;

    if (correctionActive)
    {
        // Pulsing glow effect
        float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(juce::Time::getMillisecondCounter()) * 0.006f);

        // Outer glow layers
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentYellow).withAlpha(0.15f * pulse));
        g.fillEllipse(indicatorX - 8 * scale, indicatorY - 8 * scale,
                      indicatorSize + 16 * scale, indicatorSize + 16 * scale);

        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentYellow).withAlpha(0.3f * pulse));
        g.fillEllipse(indicatorX - 4 * scale, indicatorY - 4 * scale,
                      indicatorSize + 8 * scale, indicatorSize + 8 * scale);

        // Core LED
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentYellow));
        g.fillEllipse(indicatorX, indicatorY, indicatorSize, indicatorSize);

        // Bright center
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentYellowBright));
        g.fillEllipse(indicatorX + 2 * scale, indicatorY + 2 * scale,
                      indicatorSize - 4 * scale, indicatorSize - 4 * scale);

        // Correction amount text
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentYellow));
        g.setFont(juce::FontOptions(9.0f * scale).withStyle("Bold"));
        juce::String corrText = "-" + juce::String(static_cast<int>(correctionAmount * 100)) + "%";
        g.drawText(corrText, indicatorX - 35 * scale, indicatorY - 1 * scale,
                   30 * scale, 12 * scale, juce::Justification::centredRight);
    }
    else if (autoPhaseButton.getToggleState())
    {
        // Dim green when auto phase is on but not correcting
        g.setColour(UI::NuroColors::colour(UI::NuroColors::meterGreen).withAlpha(0.15f));
        g.fillEllipse(indicatorX - 4 * scale, indicatorY - 4 * scale,
                      indicatorSize + 8 * scale, indicatorSize + 8 * scale);

        g.setColour(UI::NuroColors::colour(UI::NuroColors::meterGreen).withAlpha(0.5f));
        g.fillEllipse(indicatorX, indicatorY, indicatorSize, indicatorSize);
    }
}

void HaasAlignDelayEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float scale = getWidth() / static_cast<float>(baseWidth);

    // === BACKGROUND: Deep black with subtle gradient ===
    juce::ColourGradient bgGradient(
        UI::NuroColors::colour(UI::NuroColors::backgroundDark), 0, 0,
        UI::NuroColors::colour(UI::NuroColors::backgroundBlack), 0, bounds.getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillRect(bounds);

    // === HEADER BAR ===
    float headerHeight = 52.0f * scale;
    auto headerBounds = bounds.removeFromTop(headerHeight);

    g.setColour(UI::NuroColors::colour(UI::NuroColors::backgroundBlack));
    g.fillRect(headerBounds);

    // Header bottom line
    g.setColour(UI::NuroColors::colour(UI::NuroColors::borderMedium).withAlpha(0.5f));
    g.fillRect(0.0f, headerHeight - 1 * scale, getWidth() * 1.0f, 1.0f * scale);

    // Logo / Plugin Name
    if (logoImage.isValid())
    {
        float logoHeight = 32.0f * scale;
        float logoWidth = logoImage.getWidth() * (logoHeight / logoImage.getHeight());
        g.drawImage(logoImage,
                    static_cast<int>(20 * scale), static_cast<int>(10 * scale),
                    static_cast<int>(logoWidth), static_cast<int>(logoHeight),
                    0, 0, logoImage.getWidth(), logoImage.getHeight());
    }
    else
    {
        // Fallback text logo
        g.setFont(juce::FontOptions(24.0f * scale).withStyle("Bold"));
        g.setColour(UI::NuroColors::colour(UI::NuroColors::textWhite));
        g.drawText("HAAS", static_cast<int>(20 * scale), static_cast<int>(12 * scale),
                   static_cast<int>(70 * scale), static_cast<int>(28 * scale), juce::Justification::centredLeft);

        // "PRO" badge
        auto proBadge = juce::Rectangle<float>(90 * scale, 16 * scale, 40 * scale, 20 * scale);
        g.setColour(UI::NuroColors::colour(UI::NuroColors::accentCyan));
        g.fillRoundedRectangle(proBadge, 4.0f * scale);
        g.setColour(UI::NuroColors::colour(UI::NuroColors::backgroundBlack));
        g.setFont(juce::FontOptions(11.0f * scale).withStyle("Bold"));
        g.drawText("PRO", proBadge, juce::Justification::centred);
    }

    // === MAIN CONTENT AREA ===
    float footerHeight = 50.0f * scale;
    auto contentBounds = getLocalBounds().toFloat();
    contentBounds.removeFromTop(headerHeight);
    contentBounds.removeFromBottom(footerHeight);
    auto panelArea = contentBounds.reduced(16 * scale, 12 * scale);

    // Panel layout: 3 main knobs + controls section
    float panelSpacing = 12.0f * scale;
    float mainPanelWidth = (panelArea.getWidth() - panelSpacing * 2) * 0.28f;
    float controlPanelWidth = panelArea.getWidth() - mainPanelWidth * 3 - panelSpacing * 3;
    float panelHeight = panelArea.getHeight();

    // Draw the 4 panels
    float panelX = panelArea.getX();

    // Panel 1: LEFT DELAY
    auto panel1 = juce::Rectangle<float>(panelX, panelArea.getY(), mainPanelWidth, panelHeight);
    drawPanel(g, panel1, "LEFT DELAY");
    panelX += mainPanelWidth + panelSpacing;

    // Panel 2: WIDTH
    auto panel2 = juce::Rectangle<float>(panelX, panelArea.getY(), mainPanelWidth, panelHeight);
    drawPanel(g, panel2, "WIDTH");
    drawCorrectionIndicator(g, panel2);
    panelX += mainPanelWidth + panelSpacing;

    // Panel 3: RIGHT DELAY
    auto panel3 = juce::Rectangle<float>(panelX, panelArea.getY(), mainPanelWidth, panelHeight);
    drawPanel(g, panel3, "RIGHT DELAY");
    panelX += mainPanelWidth + panelSpacing;

    // Panel 4: CONTROLS
    auto panel4 = juce::Rectangle<float>(panelX, panelArea.getY(), controlPanelWidth, panelHeight);
    drawPanel(g, panel4, "CONTROLS");

    // === DRAW VALUE DISPLAYS ===
    float knobSize = 85.0f * scale;
    float valueDisplayWidth = 70.0f * scale;
    float valueDisplayHeight = 22.0f * scale;

    // Calculate positions for value displays (below knobs)
    auto drawKnobValue = [&](juce::Rectangle<float> panel, double value, const juce::String& unit,
                             const juce::String& subValue = {})
    {
        float centerX = panel.getCentreX();
        float valueY = panel.getY() + 28 * scale + knobSize + 15 * scale;

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
    g.setFont(juce::FontOptions(9.0f * scale));
    g.drawText("PHASE INVERT", panel4.getX() + 10 * scale, panel4.getY() + 28 * scale,
               panel4.getWidth() - 20 * scale, 12 * scale, juce::Justification::centred);

    // Mix label and value
    g.setColour(UI::NuroColors::colour(UI::NuroColors::accentCyan));
    g.setFont(juce::FontOptions(10.0f * scale).withStyle("Bold"));
    g.drawText("MIX", panel4.getX() + 10 * scale, panel4.getY() + 95 * scale,
               panel4.getWidth() - 20 * scale, 12 * scale, juce::Justification::centred);

    // Mix value display
    auto mixValueBounds = juce::Rectangle<float>(
        controlCenterX - 35 * scale, panel4.getY() + 110 * scale + 60 * scale + 8 * scale,
        70 * scale, 20 * scale);
    g.setColour(UI::NuroColors::colour(UI::NuroColors::backgroundBlack));
    g.fillRoundedRectangle(mixValueBounds, 4.0f * scale);
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textWhite));
    g.setFont(juce::FontOptions(13.0f * scale).withStyle("Bold"));
    g.drawText(juce::String(static_cast<int>(mixSlider.getValue())) + "%",
               mixValueBounds, juce::Justification::centred);

    // Auto Phase label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textMuted));
    g.setFont(juce::FontOptions(9.0f * scale));
    g.drawText("AUTO PHASE", panel4.getX() + 10 * scale, panel4.getY() + 218 * scale,
               panel4.getWidth() - 20 * scale, 12 * scale, juce::Justification::centred);

    // Safety label
    g.drawText("SAFETY", panel4.getX() + 10 * scale, panel4.getY() + 260 * scale,
               panel4.getWidth() - 20 * scale, 12 * scale, juce::Justification::centred);

    // === FOOTER BAR ===
    auto footerBounds = getLocalBounds().toFloat().removeFromBottom(footerHeight);

    g.setColour(UI::NuroColors::colour(UI::NuroColors::backgroundBlack));
    g.fillRect(footerBounds);

    // Footer top line
    g.setColour(UI::NuroColors::colour(UI::NuroColors::borderMedium).withAlpha(0.5f));
    g.fillRect(0.0f, footerBounds.getY(), getWidth() * 1.0f, 1.0f * scale);

    float meterY = footerBounds.getY() + 18 * scale;

    // Input label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textMuted));
    g.setFont(juce::FontOptions(10.0f * scale));
    g.drawText("INPUT", static_cast<int>(16 * scale), static_cast<int>(meterY - 2 * scale),
               static_cast<int>(45 * scale), static_cast<int>(14 * scale), juce::Justification::centredLeft);

    // Correlation value with color coding
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

    float corrWidth = 100.0f * scale;
    float corrX = footerBounds.getCentreX() - corrWidth / 2.0f;

    // Correlation label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textMuted));
    g.setFont(juce::FontOptions(9.0f * scale));
    g.drawText("PHASE", corrX, meterY - 14 * scale, corrWidth, 12 * scale, juce::Justification::centred);

    // Correlation value
    g.setColour(corrColor);
    g.setFont(juce::FontOptions(11.0f * scale).withStyle("Bold"));
    juce::String corrValueStr = (corr >= 0 ? "+" : "") + juce::String(corr, 2);
    g.drawText(corrValueStr, corrX, meterY + 16 * scale, corrWidth, 14 * scale, juce::Justification::centred);

    // Scale markers
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textDisabled));
    g.setFont(juce::FontOptions(8.0f * scale));
    g.drawText("-1", corrX - 12 * scale, meterY + 16 * scale, 20 * scale, 12 * scale, juce::Justification::centred);
    g.drawText("+1", corrX + corrWidth - 8 * scale, meterY + 16 * scale, 20 * scale, 12 * scale, juce::Justification::centred);

    // Output label
    g.setColour(UI::NuroColors::colour(UI::NuroColors::textMuted));
    g.setFont(juce::FontOptions(10.0f * scale));
    g.drawText("OUTPUT", static_cast<int>(footerBounds.getWidth() - 60 * scale), static_cast<int>(meterY - 2 * scale),
               static_cast<int>(50 * scale), static_cast<int>(14 * scale), juce::Justification::centredRight);
}

void HaasAlignDelayEditor::resized()
{
    auto bounds = getLocalBounds();
    float scale = getWidth() / static_cast<float>(baseWidth);

    // Scale button in bottom right corner
    int scaleButtonWidth = static_cast<int>(44 * scale);
    int scaleButtonHeight = static_cast<int>(20 * scale);
    scaleButton.setBounds(bounds.getWidth() - scaleButtonWidth - static_cast<int>(8 * scale),
                          bounds.getHeight() - scaleButtonHeight - static_cast<int>(6 * scale),
                          scaleButtonWidth, scaleButtonHeight);

    // Bypass button - top right
    bypassButton.setBounds(static_cast<int>(bounds.getWidth() - 90 * scale), static_cast<int>(14 * scale),
                           static_cast<int>(75 * scale), static_cast<int>(26 * scale));

    // Main content area
    float headerHeight = 52.0f * scale;
    float footerHeight = 50.0f * scale;
    auto contentBounds = bounds.toFloat();
    contentBounds.removeFromTop(headerHeight);
    contentBounds.removeFromBottom(footerHeight);
    auto panelArea = contentBounds.reduced(16 * scale, 12 * scale);

    // Panel dimensions
    float panelSpacing = 12.0f * scale;
    float mainPanelWidth = (panelArea.getWidth() - panelSpacing * 2) * 0.28f;
    float controlPanelWidth = panelArea.getWidth() - mainPanelWidth * 3 - panelSpacing * 3;

    // Knob size and positioning
    float knobSize = 85.0f * scale;
    float knobY = panelArea.getY() + 28 * scale;

    auto getKnobCenter = [&](int panelIndex) -> float
    {
        float pX = panelArea.getX() + panelIndex * (mainPanelWidth + panelSpacing);
        return pX + mainPanelWidth / 2.0f;
    };

    // Position main knobs
    delayLeftSlider.setBounds(static_cast<int>(getKnobCenter(0) - knobSize / 2), static_cast<int>(knobY),
                              static_cast<int>(knobSize), static_cast<int>(knobSize));
    widthSlider.setBounds(static_cast<int>(getKnobCenter(1) - knobSize / 2), static_cast<int>(knobY),
                          static_cast<int>(knobSize), static_cast<int>(knobSize));
    delayRightSlider.setBounds(static_cast<int>(getKnobCenter(2) - knobSize / 2), static_cast<int>(knobY),
                               static_cast<int>(knobSize), static_cast<int>(knobSize));

    // Controls panel positioning
    float panel4X = panelArea.getX() + 3 * (mainPanelWidth + panelSpacing);
    float panel4CenterX = panel4X + controlPanelWidth / 2.0f;

    // Phase buttons (L and R side by side)
    float phaseButtonWidth = 36.0f * scale;
    float phaseButtonHeight = 28.0f * scale;
    float phaseButtonSpacing = 8.0f * scale;
    float phaseButtonsY = panelArea.getY() + 42 * scale;

    phaseLeftButton.setBounds(static_cast<int>(panel4CenterX - phaseButtonWidth - phaseButtonSpacing / 2),
                              static_cast<int>(phaseButtonsY),
                              static_cast<int>(phaseButtonWidth), static_cast<int>(phaseButtonHeight));
    phaseRightButton.setBounds(static_cast<int>(panel4CenterX + phaseButtonSpacing / 2),
                               static_cast<int>(phaseButtonsY),
                               static_cast<int>(phaseButtonWidth), static_cast<int>(phaseButtonHeight));

    // Mix knob (smaller)
    float mixKnobSize = 60.0f * scale;
    float mixKnobY = panelArea.getY() + 110 * scale;
    mixSlider.setBounds(static_cast<int>(panel4CenterX - mixKnobSize / 2), static_cast<int>(mixKnobY),
                        static_cast<int>(mixKnobSize), static_cast<int>(mixKnobSize));

    // Auto Phase button
    float autoButtonY = panelArea.getY() + 232 * scale;
    autoPhaseButton.setBounds(static_cast<int>(panel4CenterX - 35 * scale), static_cast<int>(autoButtonY),
                              static_cast<int>(70 * scale), static_cast<int>(26 * scale));

    // Phase Safety selector
    float safetyY = panelArea.getY() + 274 * scale;
    phaseSafetySelector.setBounds(static_cast<int>(panel4CenterX - 45 * scale), static_cast<int>(safetyY),
                                  static_cast<int>(90 * scale), static_cast<int>(22 * scale));

    // Meters in footer
    float meterHeight = 14.0f * scale;
    float meterY = bounds.getHeight() - footerHeight + 18 * scale;

    inputMeter.setBounds(static_cast<int>(60 * scale), static_cast<int>(meterY),
                         static_cast<int>(140 * scale), static_cast<int>(meterHeight));

    float corrWidth = 100.0f * scale;
    float corrX = bounds.getWidth() / 2.0f - corrWidth / 2.0f;
    correlationMeter.setBounds(static_cast<int>(corrX), static_cast<int>(meterY),
                               static_cast<int>(corrWidth), static_cast<int>(meterHeight));

    outputMeter.setBounds(static_cast<int>(bounds.getWidth() - 200 * scale), static_cast<int>(meterY),
                          static_cast<int>(140 * scale), static_cast<int>(meterHeight));
}

void HaasAlignDelayEditor::timerCallback()
{
    // Update meters from processor
    inputMeter.setLevel(processorRef.getInputLevel());
    outputMeter.setLevel(processorRef.getOutputLevel());
    correlationMeter.setCorrelation(processorRef.getCorrelation());

    repaint();
}
