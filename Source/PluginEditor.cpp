#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

//==============================================================================
// HaasAlignDelayEditor - Auto Phase Correction UI
//==============================================================================
HaasAlignDelayEditor::HaasAlignDelayEditor(HaasAlignDelayProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&voxLookAndFeel);

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

    // Scale picker button (bottom-right corner, like FabFilter Pro-Q)
    scaleButton.setButtonText("100%");
    scaleButton.setClickingTogglesState(false);
    scaleButton.onClick = [this]()
    {
        // Cycle through scale presets
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

    // Start at 100% scale (preset index 2)
    currentScalePreset = 2;
    setSize(baseWidth, baseHeight);
    setResizable(false, false);  // No free-form resizing
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

    // Update button text
    int percent = static_cast<int>(scaleFactor * 100);
    scaleButton.setButtonText(juce::String(percent) + "%");

    setSize(newWidth, newHeight);
}

void HaasAlignDelayEditor::drawPanel(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    float scale = getWidth() / static_cast<float>(baseWidth);

    g.setColour(UI::VoxProLookAndFeel::panelBackground);
    g.fillRoundedRectangle(bounds, 10.0f * scale);

    g.setColour(UI::VoxProLookAndFeel::panelBorder);
    g.drawRoundedRectangle(bounds, 10.0f * scale, 1.0f * scale);
}

void HaasAlignDelayEditor::drawCorrectionIndicator(juce::Graphics& g, juce::Rectangle<float> bounds, float scale)
{
    bool correctionActive = processorRef.isCorrectionActive();
    float correctionAmount = processorRef.getCorrectionAmount();

    // Draw indicator circle in top-right of panel
    float indicatorSize = 8.0f * scale;
    float indicatorX = bounds.getRight() - indicatorSize - 6 * scale;
    float indicatorY = bounds.getY() + 6 * scale;

    if (correctionActive)
    {
        // Pulsing glow when correcting
        float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(juce::Time::getMillisecondCounter()) * 0.005f);

        // Outer glow
        g.setColour(UI::VoxProLookAndFeel::accentYellow.withAlpha(0.3f * pulse));
        g.fillEllipse(indicatorX - 4 * scale, indicatorY - 4 * scale,
                      indicatorSize + 8 * scale, indicatorSize + 8 * scale);

        // Inner indicator
        g.setColour(UI::VoxProLookAndFeel::accentYellow);
        g.fillEllipse(indicatorX, indicatorY, indicatorSize, indicatorSize);

        // Show correction percentage in top-left
        g.setColour(UI::VoxProLookAndFeel::accentYellow.withAlpha(0.8f));
        g.setFont(juce::FontOptions(8.0f * scale));
        juce::String corrText = "-" + juce::String(static_cast<int>(correctionAmount * 100)) + "%";
        g.drawText(corrText, static_cast<int>(bounds.getX() + 6 * scale), static_cast<int>(bounds.getY() + 4 * scale),
                   static_cast<int>(40 * scale), static_cast<int>(12 * scale), juce::Justification::centredLeft);
    }
    else if (autoPhaseButton.getToggleState())
    {
        // Dim green when auto phase is on but not correcting
        g.setColour(UI::VoxProLookAndFeel::meterGreen.withAlpha(0.5f));
        g.fillEllipse(indicatorX, indicatorY, indicatorSize, indicatorSize);
    }
}

void HaasAlignDelayEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float scale = getWidth() / static_cast<float>(baseWidth);

    // === XVOX STYLE: Very dark background ===
    g.fillAll(UI::VoxProLookAndFeel::backgroundDark);

    juce::ColourGradient bgGradient(
        juce::Colour(0xff0f0f14), 0, 0,
        juce::Colour(0xff080810), 0, bounds.getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillRect(bounds);

    // === HEADER ===
    float headerHeight = 50.0f * scale;
    auto headerBounds = bounds.removeFromTop(headerHeight);
    g.setColour(juce::Colour(0xff0c0c10));
    g.fillRect(headerBounds);

    // Logo
    if (logoImage.isValid())
    {
        float logoHeight = 30.0f * scale;
        float logoWidth = logoImage.getWidth() * (logoHeight / logoImage.getHeight());
        g.drawImage(logoImage,
                    static_cast<int>(16 * scale), static_cast<int>(10 * scale),
                    static_cast<int>(logoWidth), static_cast<int>(logoHeight),
                    0, 0, logoImage.getWidth(), logoImage.getHeight());
    }
    else
    {
        g.setFont(juce::FontOptions(22.0f * scale).withStyle("Bold"));
        g.setColour(UI::VoxProLookAndFeel::textWhite);
        g.drawText("HAAS", static_cast<int>(16 * scale), static_cast<int>(12 * scale),
                   static_cast<int>(70 * scale), static_cast<int>(26 * scale), juce::Justification::centredLeft);

        auto alignBox = juce::Rectangle<float>(86 * scale, 14 * scale, 60 * scale, 22 * scale);
        g.setColour(UI::VoxProLookAndFeel::accentPurple);
        g.fillRoundedRectangle(alignBox, 4.0f * scale);
        g.setColour(UI::VoxProLookAndFeel::textWhite);
        g.setFont(juce::FontOptions(12.0f * scale).withStyle("Bold"));
        g.drawText("ALIGN", alignBox, juce::Justification::centred);
    }

    g.setColour(juce::Colour(0xff1a1a22));
    g.fillRect(0.0f, headerHeight - 1 * scale, getWidth() * 1.0f, 1.0f * scale);

    // === MAIN CONTENT ===
    float footerHeight = 44.0f * scale;
    auto contentBounds = getLocalBounds().toFloat();
    contentBounds.removeFromTop(headerHeight);
    contentBounds.removeFromBottom(footerHeight);
    auto panelArea = contentBounds.reduced(12 * scale, 10 * scale);

    float panelSpacing = 8.0f * scale;
    float panelWidth = (panelArea.getWidth() - panelSpacing * 3) / 4.0f;
    float panelHeight = panelArea.getHeight();

    // Draw panels
    float panelX = panelArea.getX();
    for (int i = 0; i < 4; ++i)
    {
        auto panelRect = juce::Rectangle<float>(panelX, panelArea.getY(), panelWidth, panelHeight);
        drawPanel(g, panelRect);

        // Draw correction indicator on WIDTH panel (index 2)
        if (i == 2)
        {
            drawCorrectionIndicator(g, panelRect, scale);
        }

        panelX += panelWidth + panelSpacing;
    }

    // Knob labels and values
    float knobSize = 90.0f * scale;
    float firstPanelX = panelArea.getX();
    float knobY = panelArea.getY() + 35 * scale;

    for (int i = 0; i < 4; ++i)
    {
        float pX = firstPanelX + i * (panelWidth + panelSpacing);
        float centerX = pX + panelWidth / 2.0f;

        g.setColour(UI::VoxProLookAndFeel::accentPurple);
        g.setFont(juce::FontOptions(10.0f * scale).withStyle("Bold"));
        const char* labels[] = { "DELAY L", "DELAY R", "WIDTH", "MIX" };
        g.drawText(labels[i], static_cast<int>(centerX - 45 * scale), static_cast<int>(knobY - 22 * scale),
                   static_cast<int>(90 * scale), static_cast<int>(14 * scale), juce::Justification::centred);

        float valueY = knobY + knobSize + 8 * scale;
        g.setColour(UI::VoxProLookAndFeel::textWhite);
        g.setFont(juce::FontOptions(15.0f * scale).withStyle("Bold"));

        juce::String valueStr;
        juce::String subValueStr;
        if (i == 0) {
            valueStr = juce::String(delayLeftSlider.getValue(), 1) + " ms";
            subValueStr = juce::String(static_cast<int>(processorRef.msToSamples(static_cast<float>(delayLeftSlider.getValue())))) + " smp";
        } else if (i == 1) {
            valueStr = juce::String(delayRightSlider.getValue(), 1) + " ms";
            subValueStr = juce::String(static_cast<int>(processorRef.msToSamples(static_cast<float>(delayRightSlider.getValue())))) + " smp";
        } else if (i == 2) {
            valueStr = juce::String(static_cast<int>(widthSlider.getValue())) + "%";
            // Show effective width if correction is active
            if (processorRef.isCorrectionActive())
            {
                subValueStr = "-> " + juce::String(static_cast<int>(processorRef.getEffectiveWidth())) + "%";
            }
        } else {
            valueStr = juce::String(static_cast<int>(mixSlider.getValue())) + "%";
            subValueStr = juce::String(static_cast<int>(processorRef.getCurrentSampleRate() / 1000.0)) + " kHz";
        }

        g.drawText(valueStr, static_cast<int>(centerX - 50 * scale), static_cast<int>(valueY),
                   static_cast<int>(100 * scale), static_cast<int>(20 * scale), juce::Justification::centred);

        if (subValueStr.isNotEmpty())
        {
            // Use yellow for correction active subtext
            if (i == 2 && processorRef.isCorrectionActive())
                g.setColour(UI::VoxProLookAndFeel::accentYellow);
            else
                g.setColour(UI::VoxProLookAndFeel::textGray);

            g.setFont(juce::FontOptions(9.0f * scale));
            g.drawText(subValueStr, static_cast<int>(centerX - 50 * scale), static_cast<int>(valueY + 18 * scale),
                       static_cast<int>(100 * scale), static_cast<int>(14 * scale), juce::Justification::centred);
        }
    }

    // Connection bar between DELAY L and DELAY R
    float barCenterX = firstPanelX + panelWidth + (panelSpacing / 2.0f);
    float barTopY = panelArea.getY() + 12 * scale;
    float barBottomY = panelArea.getY() + panelHeight - 12 * scale;

    g.setColour(UI::VoxProLookAndFeel::panelBorder);
    g.fillRoundedRectangle(barCenterX - 1.5f * scale, barTopY, 3.0f * scale, barBottomY - barTopY, 1.5f * scale);

    // Phase labels
    float valueY = knobY + knobSize + 8 * scale;
    float buttonsStartY = valueY + 18 * scale;
    g.setColour(UI::VoxProLookAndFeel::textDark);
    g.setFont(juce::FontOptions(8.0f * scale));
    g.drawText("PHASE", static_cast<int>(barCenterX - 22 * scale), static_cast<int>(buttonsStartY - 12 * scale),
               static_cast<int>(44 * scale), static_cast<int>(10 * scale), juce::Justification::centred);
    g.drawText("SWAP", static_cast<int>(barCenterX - 22 * scale), static_cast<int>(buttonsStartY + 24 * scale + 6 * scale + 24 * scale + 4 * scale),
               static_cast<int>(44 * scale), static_cast<int>(10 * scale), juce::Justification::centred);

    // Auto Phase label and Safety label
    float panel3X = firstPanelX + 3 * (panelWidth + panelSpacing);
    float panel3CenterX = panel3X + panelWidth / 2.0f;
    g.setColour(UI::VoxProLookAndFeel::textDark);
    g.setFont(juce::FontOptions(8.0f * scale));
    g.drawText("AUTO PHASE", static_cast<int>(panel3CenterX - 40 * scale),
               static_cast<int>(buttonsStartY + 24 * scale + 6 * scale + 24 * scale + 8 * scale),
               static_cast<int>(80 * scale), static_cast<int>(10 * scale), juce::Justification::centred);

    g.drawText("SAFETY", static_cast<int>(panel3CenterX - 30 * scale),
               static_cast<int>(buttonsStartY + 24 * scale + 6 * scale + 24 * scale + 30 * scale + 22 * scale),
               static_cast<int>(60 * scale), static_cast<int>(10 * scale), juce::Justification::centred);

    // === FOOTER ===
    auto footerBounds = getLocalBounds().toFloat().removeFromBottom(footerHeight);

    g.setColour(juce::Colour(0xff0c0c10));
    g.fillRect(footerBounds);

    g.setColour(juce::Colour(0xff1a1a22));
    g.fillRect(footerBounds.getX(), footerBounds.getY(), footerBounds.getWidth(), 1.0f * scale);

    float meterY = footerBounds.getY() + 17 * scale;

    // Input label
    g.setColour(UI::VoxProLookAndFeel::textGray);
    g.setFont(juce::FontOptions(9.0f * scale));
    g.drawText("+0.0 dB", static_cast<int>(8 * scale), static_cast<int>(meterY - 2 * scale),
               static_cast<int>(45 * scale), static_cast<int>(14 * scale), juce::Justification::centredLeft);
    g.drawText("INPUT", static_cast<int>(54 * scale), static_cast<int>(meterY - 2 * scale),
               static_cast<int>(40 * scale), static_cast<int>(14 * scale), juce::Justification::centredLeft);

    // Correlation labels with color coding
    float corrWidth = 80.0f * scale;
    float corrX = footerBounds.getCentreX() - corrWidth / 2.0f;
    float meterHeight = 10.0f * scale;

    // Color-coded correlation value
    float corr = processorRef.getCorrelation();
    juce::Colour corrColor;
    if (corr > 0.5f)
        corrColor = UI::VoxProLookAndFeel::meterGreen;
    else if (corr > 0.3f)
        corrColor = UI::VoxProLookAndFeel::meterYellow;
    else
        corrColor = UI::VoxProLookAndFeel::meterRed;

    g.setColour(corrColor);
    g.setFont(juce::FontOptions(9.0f * scale).withStyle("Bold"));
    juce::String corrValueStr = (corr >= 0 ? "+" : "") + juce::String(corr, 2);
    g.drawText(corrValueStr, static_cast<int>(corrX), static_cast<int>(meterY - 14 * scale),
               static_cast<int>(corrWidth), static_cast<int>(12 * scale), juce::Justification::centred);

    g.setColour(UI::VoxProLookAndFeel::textDark);
    g.setFont(juce::FontOptions(7.0f * scale));
    g.drawText("-1", static_cast<int>(corrX - 10 * scale), static_cast<int>(meterY + meterHeight + 1 * scale),
               static_cast<int>(16 * scale), static_cast<int>(10 * scale), juce::Justification::centred);
    g.drawText("+1", static_cast<int>(corrX + corrWidth - 6 * scale), static_cast<int>(meterY + meterHeight + 1 * scale),
               static_cast<int>(16 * scale), static_cast<int>(10 * scale), juce::Justification::centred);

    // Output label
    g.setColour(UI::VoxProLookAndFeel::textGray);
    g.setFont(juce::FontOptions(9.0f * scale));
    g.drawText("OUTPUT", static_cast<int>(footerBounds.getWidth() - 190 * scale), static_cast<int>(meterY - 2 * scale),
               static_cast<int>(50 * scale), static_cast<int>(14 * scale), juce::Justification::centredRight);
    g.drawText("+0.0 dB", static_cast<int>(footerBounds.getWidth() - 50 * scale), static_cast<int>(meterY - 2 * scale),
               static_cast<int>(45 * scale), static_cast<int>(14 * scale), juce::Justification::centredRight);
}

void HaasAlignDelayEditor::resized()
{
    auto bounds = getLocalBounds();
    float scale = getWidth() / static_cast<float>(baseWidth);

    // Scale button in bottom right corner (like FabFilter Pro-Q)
    int scaleButtonWidth = static_cast<int>(42 * scale);
    int scaleButtonHeight = static_cast<int>(18 * scale);
    scaleButton.setBounds(bounds.getWidth() - scaleButtonWidth - static_cast<int>(6 * scale),
                          bounds.getHeight() - scaleButtonHeight - static_cast<int>(5 * scale),
                          scaleButtonWidth, scaleButtonHeight);

    // Bypass button - top right
    bypassButton.setBounds(static_cast<int>(bounds.getWidth() - 85 * scale), static_cast<int>(12 * scale),
                           static_cast<int>(70 * scale), static_cast<int>(26 * scale));

    // Main content
    float headerHeight = 50.0f * scale;
    float footerHeight = 44.0f * scale;
    auto contentBounds = bounds.toFloat();
    contentBounds.removeFromTop(headerHeight);
    contentBounds.removeFromBottom(footerHeight);
    auto panelArea = contentBounds.reduced(12 * scale, 10 * scale);

    float panelSpacing = 8.0f * scale;
    float panelWidth = (panelArea.getWidth() - panelSpacing * 3) / 4.0f;

    float knobSize = 90.0f * scale;
    float firstPanelX = panelArea.getX();
    float knobY = panelArea.getY() + 35 * scale;

    auto getKnobX = [&](int panelIndex) {
        float pX = firstPanelX + panelIndex * (panelWidth + panelSpacing);
        return static_cast<int>(pX + panelWidth / 2.0f - knobSize / 2.0f);
    };

    delayLeftSlider.setBounds(getKnobX(0), static_cast<int>(knobY), static_cast<int>(knobSize), static_cast<int>(knobSize));
    delayRightSlider.setBounds(getKnobX(1), static_cast<int>(knobY), static_cast<int>(knobSize), static_cast<int>(knobSize));
    widthSlider.setBounds(getKnobX(2), static_cast<int>(knobY), static_cast<int>(knobSize), static_cast<int>(knobSize));
    mixSlider.setBounds(getKnobX(3), static_cast<int>(knobY), static_cast<int>(knobSize), static_cast<int>(knobSize));

    // Phase buttons
    float valueY = knobY + knobSize + 8 * scale;
    float barCenterX = firstPanelX + panelWidth + panelSpacing / 2.0f;
    float buttonWidth = 32.0f * scale;
    float buttonHeight = 24.0f * scale;
    float buttonSpacing = 6.0f * scale;
    float buttonsStartY = valueY + 18 * scale;

    phaseLeftButton.setBounds(static_cast<int>(barCenterX - buttonWidth / 2.0f),
                               static_cast<int>(buttonsStartY),
                               static_cast<int>(buttonWidth), static_cast<int>(buttonHeight));
    phaseRightButton.setBounds(static_cast<int>(barCenterX - buttonWidth / 2.0f),
                                static_cast<int>(buttonsStartY + buttonHeight + buttonSpacing),
                                static_cast<int>(buttonWidth), static_cast<int>(buttonHeight));

    // Auto Phase button and Safety selector
    float panel3X = firstPanelX + 3 * (panelWidth + panelSpacing);
    float panel3CenterX = panel3X + panelWidth / 2.0f;
    float autoButtonY = buttonsStartY + buttonHeight + buttonSpacing;
    autoPhaseButton.setBounds(static_cast<int>(panel3CenterX - 32 * scale), static_cast<int>(autoButtonY),
                              static_cast<int>(64 * scale), static_cast<int>(24 * scale));

    // Phase Safety selector below AUTO button
    float safetyY = autoButtonY + 24 * scale + 22 * scale;
    phaseSafetySelector.setBounds(static_cast<int>(panel3CenterX - 40 * scale), static_cast<int>(safetyY),
                                  static_cast<int>(80 * scale), static_cast<int>(20 * scale));

    // Meters in footer
    float meterHeight = 10.0f * scale;
    float meterY = bounds.getHeight() - footerHeight + 17 * scale;

    inputMeter.setBounds(static_cast<int>(96 * scale), static_cast<int>(meterY),
                         static_cast<int>(130 * scale), static_cast<int>(meterHeight));

    float corrWidth = 80.0f * scale;
    float corrX = bounds.getWidth() / 2.0f - corrWidth / 2.0f;
    correlationMeter.setBounds(static_cast<int>(corrX), static_cast<int>(meterY),
                               static_cast<int>(corrWidth), static_cast<int>(meterHeight));

    outputMeter.setBounds(static_cast<int>(bounds.getWidth() - 136 * scale), static_cast<int>(meterY),
                          static_cast<int>(130 * scale), static_cast<int>(meterHeight));
}

void HaasAlignDelayEditor::timerCallback()
{
    // Update meters from processor
    inputMeter.setLevel(processorRef.getInputLevel());
    outputMeter.setLevel(processorRef.getOutputLevel());
    correlationMeter.setCorrelation(processorRef.getCorrelation());

    repaint();
}
