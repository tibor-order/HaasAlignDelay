#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "UI/NuroColors.h"

//==============================================================================
HaasAlignDelayEditor::HaasAlignDelayEditor(HaasAlignDelayProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&nuroLookAndFeel);

    // Setup sliders with proper styles - use properties for reliable color storage
    auto setupRotarySlider = [this](juce::Slider& slider, juce::Colour accentColor)
    {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setColour(juce::Slider::thumbColourId, accentColor);
        slider.setColour(juce::Slider::rotarySliderFillColourId, accentColor);
        slider.getProperties().set("accentColor", (juce::int64)accentColor.getARGB());
        addAndMakeVisible(slider);
    };

    auto setupVerticalSlider = [this](juce::Slider& slider, juce::Colour accentColor)
    {
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setColour(juce::Slider::thumbColourId, accentColor);
        slider.setColour(juce::Slider::trackColourId, accentColor);
        slider.getProperties().set("accentColor", (juce::int64)accentColor.getARGB());
        addAndMakeVisible(slider);
    };

    auto setupHorizontalSlider = [this](juce::Slider& slider, juce::Colour accentColor)
    {
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setColour(juce::Slider::thumbColourId, accentColor);
        slider.setColour(juce::Slider::trackColourId, accentColor);
        slider.getProperties().set("accentColor", (juce::int64)accentColor.getARGB());
        addAndMakeVisible(slider);
    };

    // Delay module sliders (pink)
    juce::Colour delayColor(0xffff66aa);
    setupVerticalSlider(delayLeftSlider, delayColor);
    setupVerticalSlider(delayRightSlider, delayColor);

    // Width module sliders (cyan)
    juce::Colour widthColor(0xff00d4ff);
    setupRotarySlider(widthSlider, widthColor);
    setupHorizontalSlider(lowCutSlider, widthColor);

    // Phase module sliders (orange)
    juce::Colour phaseColor(0xffff9933);
    setupRotarySlider(thresholdSlider, phaseColor);
    setupRotarySlider(speedSlider, phaseColor);

    // Output module sliders (green)
    juce::Colour outputColor(0xff00ff88);
    setupRotarySlider(outputGainSlider, outputColor);
    setupHorizontalSlider(mixSlider, outputColor);

    // Setup buttons
    auto setupPowerButton = [this](juce::TextButton& button)
    {
        button.setButtonText("");
        button.setClickingTogglesState(true);
        button.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        button.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(button);
    };

    setupPowerButton(delayPowerButton);
    setupPowerButton(widthPowerButton);
    setupPowerButton(autoPhasePowerButton);
    setupPowerButton(outputPowerButton);

    delayLinkButton.setButtonText("LINK");
    delayLinkButton.setClickingTogglesState(true);
    addAndMakeVisible(delayLinkButton);

    bypassButton.setButtonText("BYPASS");
    bypassButton.setClickingTogglesState(true);
    addAndMakeVisible(bypassButton);

    autoFixButton.setButtonText("AUTO FIX");
    autoFixButton.setClickingTogglesState(true);
    autoFixButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(autoFixButton);

    // Create attachments
    auto& apvts = processorRef.getAPVTS();

    delayLeftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "delayLeft", delayLeftSlider);
    delayRightAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "delayRight", delayRightSlider);
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "width", widthSlider);
    lowCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "lowCut", lowCutSlider);
    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "threshold", thresholdSlider);
    speedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "speed", speedSlider);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "outputGain", outputGainSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "mix", mixSlider);

    delayPowerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "delayPower", delayPowerButton);
    widthPowerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "widthPower", widthPowerButton);
    autoPhasePowerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "autoPhase", autoPhasePowerButton);
    outputPowerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "outputPower", outputPowerButton);
    delayLinkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "delayLink", delayLinkButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "bypass", bypassButton);

    // Setup resize triangle with size presets
    std::vector<UI::ResizeTriangle::SizePreset> presets;
    for (int i = 0; i < numPresets; ++i)
    {
        presets.push_back({sizePresets[i].name, sizePresets[i].width, sizePresets[i].height});
    }
    resizeTriangle.setSizePresets(presets);
    resizeTriangle.setCurrentPresetIndex(currentPresetIndex);
    resizeTriangle.onSizeSelected = [this](int index) { setScalePreset(index); };
    addAndMakeVisible(resizeTriangle);

    startTimerHz(30);

    // Start with Medium size
    currentPresetIndex = 1;
    currentScale = sizePresets[currentPresetIndex].scale;
    setSize(sizePresets[currentPresetIndex].width, sizePresets[currentPresetIndex].height);
}

HaasAlignDelayEditor::~HaasAlignDelayEditor()
{
    setLookAndFeel(nullptr);
    stopTimer();
}

void HaasAlignDelayEditor::timerCallback()
{
    // Update metering
    inputLevelL = processorRef.getInputLevel();
    inputLevelR = processorRef.getInputLevel();
    outputLevelL = processorRef.getOutputLevel();
    outputLevelR = processorRef.getOutputLevel();

    // Update animation phase
    waveAnimPhase += 0.08f;
    if (waveAnimPhase > juce::MathConstants<float>::twoPi)
        waveAnimPhase -= juce::MathConstants<float>::twoPi;

    // Update correction glow
    float targetGlow = processorRef.isCorrectionActive() ? 1.0f : 0.0f;
    correctionGlowIntensity += (targetGlow - correctionGlowIntensity) * 0.15f;

    repaint();
}

void HaasAlignDelayEditor::setScalePreset(int presetIndex)
{
    if (presetIndex >= 0 && presetIndex < numPresets)
    {
        currentPresetIndex = presetIndex;
        currentScale = sizePresets[presetIndex].scale;
        resizeTriangle.setCurrentPresetIndex(presetIndex);
        setSize(sizePresets[presetIndex].width, sizePresets[presetIndex].height);
    }
}

void HaasAlignDelayEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Background gradient (#1a1a1a to #0d0d0d)
    juce::ColourGradient bgGradient(
        juce::Colour(0xff1a1a1a), 0, 0,
        juce::Colour(0xff0d0d0d), 0, static_cast<float>(bounds.getHeight()),
        false);
    g.setGradientFill(bgGradient);
    g.fillRect(bounds);

    // Header (48px)
    auto headerBounds = bounds.removeFromTop(48);
    drawHeader(g, headerBounds);

    // Footer (40px)
    auto footerBounds = bounds.removeFromBottom(40);
    drawFooter(g, footerBounds);

    // Main content area with padding
    bounds = bounds.reduced(15, 15);

    // INPUT meters (50px wide)
    auto inputMeterBounds = bounds.removeFromLeft(50);
    drawInputMeters(g, inputMeterBounds);
    bounds.removeFromLeft(15);

    // OUTPUT meters (50px wide)
    auto outputMeterBounds = bounds.removeFromRight(50);
    bounds.removeFromRight(15);
    drawOutputMeters(g, outputMeterBounds);

    // Calculate module widths (flex: 1, 1, 1.3, 1)
    float totalFlex = 4.3f;
    int availableWidth = bounds.getWidth() - 45; // 3 gaps of 15px
    int moduleWidth1 = static_cast<int>(availableWidth / totalFlex);
    int moduleWidthPhase = static_cast<int>(availableWidth * 1.3f / totalFlex);

    // DELAY MODULE (Pink #ff66aa)
    delayModuleBounds = bounds.removeFromLeft(moduleWidth1);
    bounds.removeFromLeft(15);
    bool delayOn = delayPowerButton.getToggleState();
    drawDelayModule(g, delayModuleBounds, delayOn);

    // WIDTH MODULE (Cyan #00d4ff)
    widthModuleBounds = bounds.removeFromLeft(moduleWidth1);
    bounds.removeFromLeft(15);
    bool widthOn = widthPowerButton.getToggleState();
    drawWidthModule(g, widthModuleBounds, widthOn);

    // AUTO PHASE MODULE (Orange #ff9933) - THE STAR!
    autoPhaseModuleBounds = bounds.removeFromLeft(moduleWidthPhase);
    bounds.removeFromLeft(15);
    bool autoPhaseOn = autoPhasePowerButton.getToggleState();
    bool isCorrecting = processorRef.isCorrectionActive() && autoPhaseOn;
    drawAutoPhaseModule(g, autoPhaseModuleBounds, autoPhaseOn, isCorrecting);

    // OUTPUT MODULE (Green #00ff88)
    outputModuleBounds = bounds;
    bool outputOn = outputPowerButton.getToggleState();
    drawOutputModule(g, outputModuleBounds, outputOn);
}

void HaasAlignDelayEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Skip header and footer
    bounds.removeFromTop(48);
    bounds.removeFromBottom(40);
    bounds = bounds.reduced(15, 15);
    
    // Skip input meters
    bounds.removeFromLeft(50);
    bounds.removeFromLeft(15);
    
    // Skip output meters  
    bounds.removeFromRight(50);
    bounds.removeFromRight(15);
    
    // Calculate module widths
    float totalFlex = 4.3f;
    int availableWidth = bounds.getWidth() - 45;
    int moduleWidth1 = static_cast<int>(availableWidth / totalFlex);
    int moduleWidthPhase = static_cast<int>(availableWidth * 1.3f / totalFlex);
    
    // === DELAY MODULE ===
    auto delayBounds = bounds.removeFromLeft(moduleWidth1);
    bounds.removeFromLeft(15);
    
    // Delay module layout: header(40) + icon(70) + controls + linkButton(50)
    auto delayContent = delayBounds.reduced(1);
    delayContent.removeFromTop(40);  // header
    delayContent.removeFromTop(70);  // icon
    delayContent.removeFromBottom(50); // link button space
    
    // Center the sliders in the remaining space
    auto sliderArea = delayContent.reduced(20, 15);
    int sliderHeight = juce::jmin(120, sliderArea.getHeight());
    int sliderWidth = 40;
    int gap = 30;
    int totalWidth = sliderWidth * 2 + gap;
    int startX = sliderArea.getX() + (sliderArea.getWidth() - totalWidth) / 2;
    int startY = sliderArea.getY() + (sliderArea.getHeight() - sliderHeight) / 2;
    
    delayLeftSlider.setBounds(startX, startY, sliderWidth, sliderHeight);
    delayRightSlider.setBounds(startX + sliderWidth + gap, startY, sliderWidth, sliderHeight);
    
    // Power button
    delayPowerButton.setBounds(delayBounds.getX() + 13, delayBounds.getY() + 11, 20, 20);
    
    // Link button
    delayLinkButton.setBounds(delayBounds.getCentreX() - 30, delayBounds.getBottom() - 50, 60, 24);
    
    // === WIDTH MODULE ===
    auto widthBounds = bounds.removeFromLeft(moduleWidth1);
    bounds.removeFromLeft(15);
    
    auto widthContent = widthBounds.reduced(1);
    widthContent.removeFromTop(40);  // header
    widthContent.removeFromTop(70);  // icon
    
    // Width knob - centered, 90x90
    auto knobArea = widthContent.removeFromTop(130);
    widthSlider.setBounds(knobArea.getCentreX() - 45, knobArea.getY() + 10, 90, 90);
    
    // Low cut slider area
    auto lowCutArea = widthContent.reduced(20, 10);
    lowCutArea.removeFromTop(14); // label space
    lowCutSlider.setBounds(lowCutArea.getX(), lowCutArea.getY(), lowCutArea.getWidth(), 20);
    
    widthPowerButton.setBounds(widthBounds.getX() + 13, widthBounds.getY() + 11, 20, 20);
    
    // === AUTO PHASE MODULE ===
    auto phaseBounds = bounds.removeFromLeft(moduleWidthPhase);
    bounds.removeFromLeft(15);
    
    auto phaseContent = phaseBounds.reduced(1);
    phaseContent.removeFromTop(40);  // header
    phaseContent.removeFromTop(60);  // icon
    phaseContent.removeFromTop(70);  // correlation meter
    phaseContent.removeFromTop(45);  // auto fix button
    
    // Threshold and Speed knobs side by side
    auto phaseKnobs = phaseContent.reduced(15, 10);
    int knobW = phaseKnobs.getWidth() / 2;
    
    thresholdSlider.setBounds(phaseKnobs.getX() + knobW/2 - 32, phaseKnobs.getY(), 65, 65);
    speedSlider.setBounds(phaseKnobs.getX() + knobW + knobW/2 - 32, phaseKnobs.getY(), 65, 65);
    
    autoPhasePowerButton.setBounds(phaseBounds.getX() + 13, phaseBounds.getY() + 11, 20, 20);
    
    // Auto fix button - make it clickable
    auto fixArea = phaseBounds.reduced(1);
    fixArea.removeFromTop(40 + 60 + 70);
    autoFixButton.setBounds(fixArea.getCentreX() - 55, fixArea.getY() + 4, 110, 32);
    
    // === OUTPUT MODULE ===
    auto outputBounds = bounds;
    
    auto outputContent = outputBounds.reduced(1);
    outputContent.removeFromTop(40);  // header
    outputContent.removeFromTop(70);  // icon
    
    // Gain knob
    auto gainArea = outputContent.removeFromTop(130);
    outputGainSlider.setBounds(gainArea.getCentreX() - 40, gainArea.getY() + 10, 80, 80);
    
    // Mix slider
    auto mixArea = outputContent.reduced(15, 10);
    mixArea.removeFromTop(14); // label
    mixSlider.setBounds(mixArea.getX(), mixArea.getY(), mixArea.getWidth(), 20);
    
    outputPowerButton.setBounds(outputBounds.getX() + 13, outputBounds.getY() + 11, 20, 20);

    // Bypass button in header
    bypassButton.setBounds(getWidth() - 90, 10, 70, 28);

    // Resize triangle in bottom-right corner
    resizeTriangle.setBounds(getWidth() - 20, getHeight() - 20, 18, 18);
}

void HaasAlignDelayEditor::drawHeader(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Background
    juce::ColourGradient headerGrad(
        juce::Colour(0xff222222), 0, static_cast<float>(bounds.getY()),
        juce::Colour(0xff1a1a1a), 0, static_cast<float>(bounds.getBottom()), false);
    g.setGradientFill(headerGrad);
    g.fillRect(bounds);

    // Bottom border
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRect(bounds.getX(), bounds.getBottom() - 1, bounds.getWidth(), 1);

    // Logo area (left)
    auto logoArea = bounds.withWidth(200).withX(20);

    // Logo icon (circle with crosshairs)
    auto iconBounds = logoArea.removeFromLeft(32).reduced(2);
    g.setColour(juce::Colour(0xff00d4ff));
    g.drawEllipse(iconBounds.toFloat().reduced(4), 2.0f);
    g.fillEllipse(iconBounds.toFloat().withSizeKeepingCentre(10, 10));
    g.drawLine(static_cast<float>(iconBounds.getCentreX()), iconBounds.getY() + 2.0f,
               static_cast<float>(iconBounds.getCentreX()), iconBounds.getY() + 10.0f, 2.0f);
    g.drawLine(static_cast<float>(iconBounds.getCentreX()), iconBounds.getBottom() - 10.0f,
               static_cast<float>(iconBounds.getCentreX()), iconBounds.getBottom() - 2.0f, 2.0f);
    g.drawLine(iconBounds.getX() + 2.0f, static_cast<float>(iconBounds.getCentreY()),
               iconBounds.getX() + 10.0f, static_cast<float>(iconBounds.getCentreY()), 2.0f);
    g.drawLine(iconBounds.getRight() - 10.0f, static_cast<float>(iconBounds.getCentreY()),
               iconBounds.getRight() - 2.0f, static_cast<float>(iconBounds.getCentreY()), 2.0f);

    // Logo text "HAASFX PRO" with gradient
    auto textBounds = logoArea.withX(iconBounds.getRight() + 8);
    juce::ColourGradient logoGrad(
        juce::Colour(0xff00d4ff), static_cast<float>(textBounds.getX()), 0,
        juce::Colour(0xff00ff88), static_cast<float>(textBounds.getX() + 80), 0, false);
    g.setGradientFill(logoGrad);
    g.setFont(juce::FontOptions(18.0f).withStyle("Bold"));
    g.drawText("HAAS", textBounds, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff00d4ff).withAlpha(0.6f));
    g.drawText("FX", textBounds.withX(textBounds.getX() + 52), juce::Justification::centredLeft);

    // PRO badge
    auto proBadge = juce::Rectangle<int>(textBounds.getX() + 80, bounds.getCentreY() - 9, 32, 18);
    g.setColour(juce::Colour(0xff00d4ff).withAlpha(0.15f));
    g.fillRoundedRectangle(proBadge.toFloat(), 3.0f);
    g.setColour(juce::Colour(0xff00d4ff));
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.drawText("PRO", proBadge, juce::Justification::centred);

    // Preset selector (center)
    auto presetArea = bounds.withSizeKeepingCentre(200, 32);
    g.setColour(juce::Colour(0xff0d0d0d));
    g.fillRoundedRectangle(presetArea.toFloat(), 6.0f);
    g.setColour(juce::Colour(0xff333333));
    g.drawRoundedRectangle(presetArea.toFloat(), 6.0f, 1.0f);

    g.setColour(juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(14.0f));
    g.drawText("<", presetArea.removeFromLeft(25), juce::Justification::centred);
    g.drawText(">", presetArea.removeFromRight(25), juce::Justification::centred);

    g.setColour(juce::Colour(0xffcccccc));
    g.setFont(juce::FontOptions(13.0f));
    g.drawText("Clean Stereo", presetArea, juce::Justification::centred);

    // Bypass button (right)
    bool bypassed = bypassButton.getToggleState();
    auto bypassBounds = juce::Rectangle<int>(bounds.getRight() - 90, bounds.getCentreY() - 14, 70, 28);
    g.setColour(bypassed ? juce::Colour(0xffff3366) : juce::Colour(0xff2a2a2a));
    g.fillRoundedRectangle(bypassBounds.toFloat(), 6.0f);
    g.setColour(bypassed ? juce::Colour(0xffff3366) : juce::Colour(0xff444444));
    g.drawRoundedRectangle(bypassBounds.toFloat(), 6.0f, 1.0f);
    g.setColour(bypassed ? juce::Colours::white : juce::Colour(0xff888888));
    g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    g.drawText("BYPASS", bypassBounds, juce::Justification::centred);
}

void HaasAlignDelayEditor::drawFooter(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour(juce::Colour(0xff0d0d0d));
    g.fillRect(bounds);

    // Top border
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRect(bounds.getX(), bounds.getY(), bounds.getWidth(), 1);

    // REORDER AUDIO text
    g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    auto textBounds = bounds.withSizeKeepingCentre(150, 20);

    g.setColour(juce::Colour(0xff444444));
    g.drawText("REORDER", textBounds.withWidth(60), juce::Justification::centredRight);

    g.setColour(juce::Colour(0xff00d4ff));
    g.drawText("AUDIO", textBounds.withX(textBounds.getCentreX() + 5).withWidth(50), juce::Justification::centredLeft);
}

void HaasAlignDelayEditor::drawDelayModule(juce::Graphics& g, juce::Rectangle<int> bounds, bool isOn)
{
    juce::Colour accentColor(0xffff66aa);

    // Module background
    g.setColour(juce::Colour(0xff141414));
    g.fillRoundedRectangle(bounds.toFloat(), 10.0f);
    g.setColour(juce::Colour(0xff2a2a2a));
    g.drawRoundedRectangle(bounds.toFloat(), 10.0f, 1.0f);

    auto content = bounds.reduced(1);

    // Header
    auto header = content.removeFromTop(40);
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRect(header.getX(), header.getBottom() - 1, header.getWidth(), 1);

    // Power button is drawn by JUCE, just draw background
    auto powerBounds = juce::Rectangle<int>(header.getX() + 12, header.getCentreY() - 10, 20, 20);
    drawPowerButton(g, powerBounds, accentColor, isOn);

    // Title
    g.setColour(accentColor);
    g.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
    g.drawText("DELAY", header.withX(powerBounds.getRight() + 8), juce::Justification::centredLeft);

    // Icon area
    auto iconArea = content.removeFromTop(70);
    drawWaveIcon(g, iconArea.withSizeKeepingCentre(60, 60), accentColor, isOn);

    // Slider labels (sliders are drawn by LookAndFeel)
    auto slidersArea = content.reduced(15, 10);
    slidersArea.removeFromBottom(40);
    
    int sliderWidth = (slidersArea.getWidth() - 20) / 2;
    auto leftArea = slidersArea.removeFromLeft(sliderWidth);
    slidersArea.removeFromLeft(20);
    auto rightArea = slidersArea;

    // Value labels
    g.setColour(isOn ? accentColor : juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    g.drawText(juce::String(delayLeftSlider.getValue(), 1) + "ms", 
               leftArea.removeFromBottom(18), juce::Justification::centred);
    g.drawText(juce::String(delayRightSlider.getValue(), 1) + "ms",
               rightArea.removeFromBottom(18), juce::Justification::centred);

    // Channel labels
    g.setColour(juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("LEFT", leftArea.removeFromBottom(14), juce::Justification::centred);
    g.drawText("RIGHT", rightArea.removeFromBottom(14), juce::Justification::centred);

    // Link button background (button is drawn by JUCE)
    auto linkBounds = bounds.withHeight(24).withY(bounds.getBottom() - 50).withSizeKeepingCentre(60, 24);
    bool linked = delayLinkButton.getToggleState();
    g.setColour(linked ? accentColor.withAlpha(0.2f) : juce::Colour(0xff1a1a1a));
    g.fillRoundedRectangle(linkBounds.toFloat(), 4.0f);
    g.setColour(linked ? accentColor : juce::Colour(0xff333333));
    g.drawRoundedRectangle(linkBounds.toFloat(), 4.0f, 1.0f);
}

void HaasAlignDelayEditor::drawWidthModule(juce::Graphics& g, juce::Rectangle<int> bounds, bool isOn)
{
    juce::Colour accentColor(0xff00d4ff);

    // Module background
    g.setColour(juce::Colour(0xff141414));
    g.fillRoundedRectangle(bounds.toFloat(), 10.0f);
    g.setColour(juce::Colour(0xff2a2a2a));
    g.drawRoundedRectangle(bounds.toFloat(), 10.0f, 1.0f);

    auto content = bounds.reduced(1);

    // Header
    auto header = content.removeFromTop(40);
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRect(header.getX(), header.getBottom() - 1, header.getWidth(), 1);

    auto powerBounds = juce::Rectangle<int>(header.getX() + 12, header.getCentreY() - 10, 20, 20);
    drawPowerButton(g, powerBounds, accentColor, isOn);

    g.setColour(accentColor);
    g.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
    g.drawText("WIDTH", header.withX(powerBounds.getRight() + 8), juce::Justification::centredLeft);

    // Icon area
    auto iconArea = content.removeFromTop(70);
    drawExpandIcon(g, iconArea.withSizeKeepingCentre(60, 60), accentColor, isOn);

    // Width knob label (knob is drawn by LookAndFeel)
    auto knobArea = content.removeFromTop(120);
    
    // Value display below knob
    auto valueBox = knobArea.withSizeKeepingCentre(70, 22).translated(0, 35);
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRoundedRectangle(valueBox.toFloat(), 4.0f);
    g.setColour(juce::Colour(0xff333333));
    g.drawRoundedRectangle(valueBox.toFloat(), 4.0f, 1.0f);
    g.setColour(isOn ? accentColor : juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
    g.drawText(juce::String(static_cast<int>(widthSlider.getValue())) + "%", valueBox, juce::Justification::centred);

    // Label
    g.setColour(juce::Colour(0xff888888));
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("STEREO WIDTH", knobArea.withY(valueBox.getBottom() + 2).withHeight(14), juce::Justification::centred);

    // Low cut slider label
    auto lowCutArea = content.reduced(20, 10);
    g.setColour(isOn ? juce::Colour(0xff666666) : juce::Colour(0xff444444));
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("LOW CUT", lowCutArea.removeFromTop(14), juce::Justification::centredLeft);

    lowCutArea.removeFromTop(20); // space for slider

    g.setColour(isOn ? accentColor : juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    g.drawText(juce::String(static_cast<int>(lowCutSlider.getValue())) + "Hz",
               lowCutArea.removeFromTop(16), juce::Justification::centredRight);
}

void HaasAlignDelayEditor::drawAutoPhaseModule(juce::Graphics& g, juce::Rectangle<int> bounds,
                                                bool isOn, bool isCorrecting)
{
    juce::Colour accentColor(0xffff9933);
    juce::Colour glowColor(0xffff6600);

    // Multi-layer glow effect when correcting
    if (isCorrecting)
    {
        float pulseIntensity = 0.5f + std::sin(waveAnimPhase * 2.0f) * 0.3f;
        for (int i = 4; i >= 1; --i)
        {
            float alpha = (0.08f / i) * correctionGlowIntensity * pulseIntensity;
            g.setColour(glowColor.withAlpha(alpha));
            g.drawRoundedRectangle(bounds.toFloat().expanded(static_cast<float>(i * 3)), 12.0f, 2.0f);
        }

        g.setColour(glowColor.withAlpha(0.1f * correctionGlowIntensity));
        g.fillRoundedRectangle(bounds.expanded(2).toFloat(), 11.0f);
    }

    // Module background
    g.setColour(isCorrecting ? juce::Colour(0xff1a1410) : juce::Colour(0xff141414));
    g.fillRoundedRectangle(bounds.toFloat(), 10.0f);

    // Border
    if (isCorrecting)
    {
        g.setColour(glowColor.withAlpha(0.5f * correctionGlowIntensity));
        g.drawRoundedRectangle(bounds.toFloat(), 10.0f, 2.0f);
    }
    else
    {
        g.setColour(juce::Colour(0xff2a2a2a));
        g.drawRoundedRectangle(bounds.toFloat(), 10.0f, 1.0f);
    }

    auto content = bounds.reduced(1);

    // Header
    auto header = content.removeFromTop(40);
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRect(header.getX(), header.getBottom() - 1, header.getWidth(), 1);

    auto powerBounds = juce::Rectangle<int>(header.getX() + 12, header.getCentreY() - 10, 20, 20);
    drawPowerButton(g, powerBounds, accentColor, isOn);

    g.setColour(accentColor);
    g.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
    g.drawText("AUTO PHASE", header.withX(powerBounds.getRight() + 8), juce::Justification::centredLeft);

    // EXCLUSIVE badge
    auto badgeBounds = juce::Rectangle<int>(header.getRight() - 70, header.getCentreY() - 9, 58, 18);
    g.setColour(accentColor.withAlpha(0.15f));
    g.fillRoundedRectangle(badgeBounds.toFloat(), 3.0f);
    g.setColour(accentColor);
    g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
    g.drawText("EXCLUSIVE", badgeBounds, juce::Justification::centred);

    // Icon area
    auto iconArea = content.removeFromTop(60);
    drawPhaseIcon(g, iconArea.withSizeKeepingCentre(60, 60), accentColor, isOn, isCorrecting);

    // Correlation meter
    auto meterArea = content.removeFromTop(70);
    float correlation = processorRef.getCorrelation();
    float threshold = static_cast<float>(thresholdSlider.getValue());
    drawCorrelationMeter(g, meterArea.reduced(15, 5), correlation, threshold, isOn);

    // Auto Fix button background (button is positioned by resized)
    auto fixBounds = content.removeFromTop(40).withSizeKeepingCentre(110, 32);

    if (isCorrecting)
    {
        g.setColour(accentColor.withAlpha(0.3f));
        g.fillRoundedRectangle(fixBounds.expanded(4).toFloat(), 10.0f);
    }

    if (isOn)
    {
        juce::ColourGradient btnGrad(
            accentColor, 0, static_cast<float>(fixBounds.getY()),
            juce::Colour(0xffff6600), 0, static_cast<float>(fixBounds.getBottom()), false);
        g.setGradientFill(btnGrad);
    }
    else
    {
        g.setColour(juce::Colour(0xff1a1a1a));
    }
    g.fillRoundedRectangle(fixBounds.toFloat(), 6.0f);
    g.setColour(isOn ? accentColor : juce::Colour(0xff333333));
    g.drawRoundedRectangle(fixBounds.toFloat(), 6.0f, 1.0f);

    g.setColour(isOn ? juce::Colours::black : juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    g.drawText(isCorrecting ? "CORRECTING" : (isOn ? "AUTO FIX ON" : "AUTO FIX OFF"),
               fixBounds, juce::Justification::centred);

    // Threshold and Speed knob labels (knobs drawn by LookAndFeel)
    auto knobsArea = content.reduced(10, 10);
    int knobW = knobsArea.getWidth() / 2;

    auto threshArea = knobsArea.removeFromLeft(knobW);
    auto speedArea = knobsArea;

    // Value boxes
    auto threshValueBox = threshArea.withSizeKeepingCentre(55, 20).translated(0, 35);
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRoundedRectangle(threshValueBox.toFloat(), 4.0f);
    g.setColour(juce::Colour(0xff333333));
    g.drawRoundedRectangle(threshValueBox.toFloat(), 4.0f, 1.0f);
    g.setColour(isOn ? accentColor : juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    g.drawText(juce::String(thresholdSlider.getValue(), 2), threshValueBox, juce::Justification::centred);

    auto speedValueBox = speedArea.withSizeKeepingCentre(55, 20).translated(0, 35);
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRoundedRectangle(speedValueBox.toFloat(), 4.0f);
    g.setColour(juce::Colour(0xff333333));
    g.drawRoundedRectangle(speedValueBox.toFloat(), 4.0f, 1.0f);
    g.setColour(isOn ? accentColor : juce::Colour(0xff666666));
    g.drawText(juce::String(static_cast<int>(speedSlider.getValue())) + "%", speedValueBox, juce::Justification::centred);

    // Labels
    g.setColour(juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(9.0f));
    g.drawText("THRESHOLD", threshValueBox.translated(0, 18).withHeight(12), juce::Justification::centred);
    g.drawText("SPEED", speedValueBox.translated(0, 18).withHeight(12), juce::Justification::centred);
}

void HaasAlignDelayEditor::drawOutputModule(juce::Graphics& g, juce::Rectangle<int> bounds, bool isOn)
{
    juce::Colour accentColor(0xff00ff88);

    g.setColour(juce::Colour(0xff141414));
    g.fillRoundedRectangle(bounds.toFloat(), 10.0f);
    g.setColour(juce::Colour(0xff2a2a2a));
    g.drawRoundedRectangle(bounds.toFloat(), 10.0f, 1.0f);

    auto content = bounds.reduced(1);

    // Header
    auto header = content.removeFromTop(40);
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRect(header.getX(), header.getBottom() - 1, header.getWidth(), 1);

    auto powerBounds = juce::Rectangle<int>(header.getX() + 12, header.getCentreY() - 10, 20, 20);
    drawPowerButton(g, powerBounds, accentColor, isOn);

    g.setColour(accentColor);
    g.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
    g.drawText("OUTPUT", header.withX(powerBounds.getRight() + 8), juce::Justification::centredLeft);

    // Icon area
    auto iconArea = content.removeFromTop(70);
    drawSpeakerIcon(g, iconArea.withSizeKeepingCentre(60, 60), accentColor, isOn);

    // Gain knob label (knob drawn by LookAndFeel)
    auto knobArea = content.removeFromTop(120);
    
    // Value display
    auto valueBox = knobArea.withSizeKeepingCentre(70, 22).translated(0, 35);
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRoundedRectangle(valueBox.toFloat(), 4.0f);
    g.setColour(juce::Colour(0xff333333));
    g.drawRoundedRectangle(valueBox.toFloat(), 4.0f, 1.0f);
    
    juce::String gainStr = (outputGainSlider.getValue() >= 0 ? "+" : "") +
                           juce::String(outputGainSlider.getValue(), 1) + "dB";
    g.setColour(isOn ? accentColor : juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
    g.drawText(gainStr, valueBox, juce::Justification::centred);

    // Label
    g.setColour(juce::Colour(0xff888888));
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("GAIN", knobArea.withY(valueBox.getBottom() + 2).withHeight(14), juce::Justification::centred);

    // Dry/Wet slider label
    auto mixArea = content.reduced(15, 10);
    g.setColour(isOn ? juce::Colour(0xff666666) : juce::Colour(0xff444444));
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("DRY/WET", mixArea.removeFromTop(14), juce::Justification::centredLeft);

    mixArea.removeFromTop(20); // space for slider

    g.setColour(isOn ? accentColor : juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    g.drawText(juce::String(static_cast<int>(mixSlider.getValue())) + "%",
               mixArea.removeFromTop(16), juce::Justification::centredRight);
}

void HaasAlignDelayEditor::drawInputMeters(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Background
    g.setColour(juce::Colour(0xff0d0d0d));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    g.setColour(juce::Colour(0xff2a2a2a));
    g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 1.0f);

    auto content = bounds.reduced(8);

    // Label (vertical)
    g.setColour(juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));

    // Draw vertical text
    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText(juce::FontOptions(9.0f), "INPUT", 0, 0);

    juce::Path textPath;
    glyphs.createPath(textPath);

    auto textBounds = textPath.getBounds();
    juce::AffineTransform transform = juce::AffineTransform::rotation(-juce::MathConstants<float>::halfPi)
        .translated(content.getX() + 10, content.getCentreY() + textBounds.getWidth() / 2);
    g.fillPath(textPath, transform);

    // Meters
    auto metersArea = content.withTrimmedLeft(18);
    int meterW = (metersArea.getWidth() - 6) / 2;

    auto leftMeter = metersArea.removeFromLeft(meterW);
    metersArea.removeFromLeft(6);
    auto rightMeter = metersArea.removeFromLeft(meterW);

    drawSegmentedMeter(g, leftMeter, inputLevelL, "L");
    drawSegmentedMeter(g, rightMeter, inputLevelR, "R");

    // dB readout
    g.setColour(juce::Colour(0xff00ff88));
    g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
    g.drawText("0.0dB", bounds.removeFromBottom(20), juce::Justification::centred);
}

void HaasAlignDelayEditor::drawOutputMeters(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour(juce::Colour(0xff0d0d0d));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    g.setColour(juce::Colour(0xff2a2a2a));
    g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 1.0f);

    auto content = bounds.reduced(8);

    // Label (vertical)
    g.setColour(juce::Colour(0xff666666));
    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText(juce::FontOptions(9.0f), "OUTPUT", 0, 0);

    juce::Path textPath;
    glyphs.createPath(textPath);

    auto textBounds = textPath.getBounds();
    juce::AffineTransform transform = juce::AffineTransform::rotation(-juce::MathConstants<float>::halfPi)
        .translated(content.getX() + 10, content.getCentreY() + textBounds.getWidth() / 2);
    g.fillPath(textPath, transform);

    // Meters
    auto metersArea = content.withTrimmedLeft(18);
    int meterW = (metersArea.getWidth() - 6) / 2;

    auto leftMeter = metersArea.removeFromLeft(meterW);
    metersArea.removeFromLeft(6);
    auto rightMeter = metersArea.removeFromLeft(meterW);

    drawSegmentedMeter(g, leftMeter, outputLevelL, "L");
    drawSegmentedMeter(g, rightMeter, outputLevelR, "R");

    // dB readout
    float gain = static_cast<float>(outputGainSlider.getValue());
    juce::String gainStr = (gain >= 0 ? "+" : "") + juce::String(gain, 1) + "dB";
    g.setColour(juce::Colour(0xff00ff88));
    g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
    g.drawText(gainStr, bounds.removeFromBottom(20), juce::Justification::centred);
}

void HaasAlignDelayEditor::drawSegmentedMeter(juce::Graphics& g, juce::Rectangle<int> bounds,
                                               float level, const juce::String& label)
{
    const int numSegments = 24;
    const float segmentHeight = 3.0f;
    const float segmentGap = 2.0f;
    const float segmentWidth = 10.0f;

    float totalHeight = numSegments * (segmentHeight + segmentGap);
    float startY = bounds.getCentreY() + totalHeight / 2 - 10;
    float x = bounds.getCentreX() - segmentWidth / 2;

    // Convert level to active segments
    float dbLevel = juce::Decibels::gainToDecibels(level, -60.0f);
    float normalized = juce::jmap(dbLevel, -60.0f, 0.0f, 0.0f, 1.0f);
    int activeSegments = static_cast<int>(normalized * numSegments);

    for (int i = 0; i < numSegments; ++i)
    {
        float y = startY - (i + 1) * (segmentHeight + segmentGap);
        juce::Rectangle<float> segment(x, y, segmentWidth, segmentHeight);

        bool isActive = i < activeSegments;

        // Determine color based on position
        juce::Colour segmentColor;
        float ratio = static_cast<float>(i) / numSegments;
        if (ratio >= 0.75f)
            segmentColor = juce::Colour(0xffff3366); // Red
        else if (ratio >= 0.6f)
            segmentColor = juce::Colour(0xffffcc00); // Yellow
        else
            segmentColor = juce::Colour(0xff00ff88); // Green

        if (isActive)
        {
            // Glow behind
            g.setColour(segmentColor.withAlpha(0.4f));
            g.fillRoundedRectangle(segment.expanded(1, 0), 1.5f);

            // Main segment
            g.setColour(segmentColor);
            g.fillRoundedRectangle(segment, 1.0f);
        }
        else
        {
            g.setColour(juce::Colour(0xff222222));
            g.fillRoundedRectangle(segment, 1.0f);
        }
    }

    // Label
    g.setColour(juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(9.0f));
    g.drawText(label, bounds.removeFromBottom(15), juce::Justification::centred);
}

void HaasAlignDelayEditor::drawKnob(juce::Graphics& g, juce::Rectangle<int> bounds,
                                     const juce::String& label, const juce::String& value,
                                     juce::Colour accentColor, float normalizedValue, bool isActive)
{
    auto knobBounds = bounds.reduced(10).withTrimmedBottom(40);
    float cx = static_cast<float>(knobBounds.getCentreX());
    float cy = static_cast<float>(knobBounds.getCentreY());
    float radius = juce::jmin(knobBounds.getWidth(), knobBounds.getHeight()) / 2.0f;

    // 1. OUTER RING with value arc
    float startAngle = juce::MathConstants<float>::pi * 1.25f;
    float endAngle = juce::MathConstants<float>::pi * 2.75f;
    float valueAngle = startAngle + normalizedValue * (endAngle - startAngle);

    // Background arc (dark)
    juce::Path bgArc;
    bgArc.addCentredArc(cx, cy, radius, radius, 0, startAngle, endAngle, true);
    g.setColour(juce::Colour(0xff222222));
    g.strokePath(bgArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Value arc (colored) with glow
    if (isActive && normalizedValue > 0.01f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(cx, cy, radius, radius, 0, startAngle, valueAngle, true);

        // Glow
        g.setColour(accentColor.withAlpha(0.3f));
        g.strokePath(valueArc, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Main arc
        g.setColour(accentColor.withAlpha(0.8f));
        g.strokePath(valueArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 2. KNOB BODY - 3D metallic gradient
    float bodyRadius = radius - 8;

    // Drop shadow
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillEllipse(cx - bodyRadius + 2, cy - bodyRadius + 4, bodyRadius * 2, bodyRadius * 2);

    // Main body gradient (3D effect)
    juce::ColourGradient bodyGradient(
        juce::Colour(0xff3a3a3a), cx - bodyRadius * 0.5f, cy - bodyRadius * 0.5f,
        juce::Colour(0xff1a1a1a), cx + bodyRadius * 0.5f, cy + bodyRadius * 0.5f, true);
    g.setGradientFill(bodyGradient);
    g.fillEllipse(cx - bodyRadius, cy - bodyRadius, bodyRadius * 2, bodyRadius * 2);

    // Border
    g.setColour(juce::Colour(0xff333333));
    g.drawEllipse(cx - bodyRadius, cy - bodyRadius, bodyRadius * 2, bodyRadius * 2, 2.0f);

    // Top highlight
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawEllipse(cx - bodyRadius + 2, cy - bodyRadius + 2, bodyRadius * 2 - 4, bodyRadius * 2 - 4, 1.0f);

    // 3. CENTER CAP
    float capRadius = bodyRadius * 0.35f;
    juce::ColourGradient capGradient(
        juce::Colour(0xff444444), cx, cy - capRadius,
        juce::Colour(0xff222222), cx, cy + capRadius, false);
    g.setGradientFill(capGradient);
    g.fillEllipse(cx - capRadius, cy - capRadius, capRadius * 2, capRadius * 2);

    // 4. INDICATOR LINE
    float angle = startAngle + normalizedValue * (endAngle - startAngle) - juce::MathConstants<float>::halfPi;
    float lineLength = bodyRadius - 8;
    float lineStartRadius = capRadius + 2;

    float x1 = cx + std::cos(angle) * lineStartRadius;
    float y1 = cy + std::sin(angle) * lineStartRadius;
    float x2 = cx + std::cos(angle) * lineLength;
    float y2 = cy + std::sin(angle) * lineLength;

    // Glow
    if (isActive)
    {
        g.setColour(accentColor.withAlpha(0.4f));
        g.drawLine(x1, y1, x2, y2, 6.0f);
    }

    // Main line
    g.setColour(isActive ? accentColor : juce::Colour(0xff666666));
    g.drawLine(x1, y1, x2, y2, 2.5f);

    // Indicator tip dot
    if (isActive)
    {
        g.setColour(accentColor);
        g.fillEllipse(x2 - 3, y2 - 3, 6, 6);
    }

    // 5. VALUE DISPLAY BOX
    auto valueBox = bounds.removeFromBottom(35).reduced(15, 0).withHeight(22);
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRoundedRectangle(valueBox.toFloat(), 4.0f);
    g.setColour(juce::Colour(0xff333333));
    g.drawRoundedRectangle(valueBox.toFloat(), 4.0f, 1.0f);

    g.setColour(isActive ? accentColor : juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
    g.drawText(value, valueBox, juce::Justification::centred);

    // 6. LABEL
    g.setColour(juce::Colour(0xff888888));
    g.setFont(juce::FontOptions(10.0f));
    g.drawText(label.toUpperCase(), bounds.removeFromBottom(15), juce::Justification::centred);
}

void HaasAlignDelayEditor::drawVerticalSlider(juce::Graphics& g, juce::Rectangle<int> bounds,
                                               const juce::String& label, const juce::String& value,
                                               juce::Colour accentColor, float normalizedValue, bool isActive)
{
    auto sliderBounds = bounds.reduced(20, 10).withTrimmedBottom(50);
    float trackWidth = 12.0f;
    float trackX = sliderBounds.getCentreX() - trackWidth / 2;
    float trackHeight = static_cast<float>(sliderBounds.getHeight());
    float trackY = static_cast<float>(sliderBounds.getY());

    // 1. TRACK BACKGROUND
    juce::Rectangle<float> trackRect(trackX, trackY, trackWidth, trackHeight);

    // Inset shadow
    g.setColour(juce::Colour(0xff0a0a0a));
    g.fillRoundedRectangle(trackRect, 6.0f);
    g.setColour(juce::Colour(0xff333333));
    g.drawRoundedRectangle(trackRect, 6.0f, 1.0f);

    // 2. FILL (from bottom)
    float fillHeight = trackHeight * normalizedValue;
    if (isActive && fillHeight > 2)
    {
        juce::Rectangle<float> fillRect(trackX + 3, trackY + trackHeight - fillHeight, 6.0f, fillHeight);

        // Gradient fill
        juce::ColourGradient fillGradient(
            accentColor.withAlpha(0.5f), fillRect.getX(), fillRect.getBottom(),
            accentColor, fillRect.getX(), fillRect.getY(), false);
        g.setGradientFill(fillGradient);
        g.fillRoundedRectangle(fillRect, 3.0f);

        // Glow
        g.setColour(accentColor.withAlpha(0.4f));
        g.fillRoundedRectangle(fillRect.expanded(2, 0), 4.0f);
    }

    // 3. HANDLE/THUMB
    float handleY = trackY + trackHeight * (1.0f - normalizedValue) - 8;
    juce::Rectangle<float> handleRect(trackX - 4, handleY, 20.0f, 16.0f);

    // Handle gradient
    juce::ColourGradient handleGradient(
        juce::Colour(0xff555555), handleRect.getX(), handleRect.getY(),
        juce::Colour(0xff333333), handleRect.getX(), handleRect.getBottom(), false);
    g.setGradientFill(handleGradient);
    g.fillRoundedRectangle(handleRect, 3.0f);

    // Handle border
    g.setColour(juce::Colour(0xff444444));
    g.drawRoundedRectangle(handleRect, 3.0f, 1.0f);

    // Handle highlight
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawLine(handleRect.getX() + 3, handleRect.getY() + 2,
               handleRect.getRight() - 3, handleRect.getY() + 2, 1.0f);

    // 4. VALUE TEXT
    auto valueArea = bounds.removeFromBottom(45);
    g.setColour(isActive ? accentColor : juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
    g.drawText(value, valueArea.removeFromTop(20), juce::Justification::centred);

    // 5. LABEL
    g.setColour(juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(10.0f));
    g.drawText(label.toUpperCase(), valueArea, juce::Justification::centred);
}

void HaasAlignDelayEditor::drawCorrelationMeter(juce::Graphics& g, juce::Rectangle<int> bounds,
                                                 float correlation, float threshold, bool isActive)
{
    auto scaleBounds = bounds.removeFromTop(14);
    auto meterBounds = bounds.removeFromTop(24);
    auto valueBounds = bounds;

    // Scale labels
    g.setColour(juce::Colour(0xff666666));
    g.setFont(juce::FontOptions(9.0f));
    g.drawText("-1", scaleBounds.removeFromLeft(20), juce::Justification::centredLeft);
    g.drawText("0", scaleBounds.withSizeKeepingCentre(20, 14), juce::Justification::centred);
    g.drawText("+1", scaleBounds.removeFromRight(20), juce::Justification::centredRight);

    // Meter background
    g.setColour(juce::Colour(0xff0a0a0a));
    g.fillRoundedRectangle(meterBounds.toFloat(), 4.0f);
    g.setColour(juce::Colour(0xff333333));
    g.drawRoundedRectangle(meterBounds.toFloat(), 4.0f, 1.0f);

    // Gradient background
    juce::ColourGradient bgGrad(
        juce::Colour(0xffff3366), static_cast<float>(meterBounds.getX()), 0,
        juce::Colour(0xff00ff88), static_cast<float>(meterBounds.getRight()), 0, false);
    bgGrad.addColour(0.25, juce::Colour(0xffff6633));
    bgGrad.addColour(0.5, juce::Colour(0xffffcc00));
    bgGrad.addColour(0.75, juce::Colour(0xff66ff66));
    g.setGradientFill(bgGrad);
    g.setOpacity(0.3f);
    g.fillRoundedRectangle(meterBounds.reduced(1).toFloat(), 3.0f);
    g.setOpacity(1.0f);

    // Threshold marker
    float threshX = meterBounds.getX() + (threshold + 1.0f) / 2.0f * meterBounds.getWidth();
    g.setColour(isActive ? juce::Colour(0xffff6600) : juce::Colour(0xff444444));
    g.drawVerticalLine(static_cast<int>(threshX), static_cast<float>(meterBounds.getY()),
                       static_cast<float>(meterBounds.getBottom()));
    if (isActive)
    {
        g.setColour(juce::Colour(0xffff6600).withAlpha(0.3f));
        g.fillRect(static_cast<int>(threshX) - 2, meterBounds.getY(), 4, meterBounds.getHeight());
    }

    // Correlation position
    float normalizedCorr = (correlation + 1.0f) / 2.0f;
    float corrX = meterBounds.getX() + normalizedCorr * meterBounds.getWidth();

    // Color based on correlation
    juce::Colour corrColor;
    if (correlation > 0.5f)
        corrColor = juce::Colour(0xff00ff88);
    else if (correlation > 0.0f)
        corrColor = juce::Colour(0xffffcc00);
    else
        corrColor = juce::Colour(0xffff3366);

    // Indicator
    g.setColour(corrColor);
    g.fillRoundedRectangle(corrX - 2, static_cast<float>(meterBounds.getY() + 4),
                           4, static_cast<float>(meterBounds.getHeight() - 8), 2.0f);
    g.setColour(juce::Colours::white);
    g.fillEllipse(corrX - 3, meterBounds.getCentreY() - 3.0f, 6, 6);

    // Glow
    g.setColour(corrColor.withAlpha(0.5f));
    g.fillEllipse(corrX - 6, meterBounds.getCentreY() - 6.0f, 12, 12);

    // Value display
    g.setColour(corrColor);
    g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
    juce::String corrStr = (correlation >= 0 ? "+" : "") + juce::String(correlation, 2);
    g.drawText(corrStr, valueBounds.removeFromTop(18), juce::Justification::centred);

    // Correcting indicator
    if (isActive && correlation < threshold)
    {
        auto indicatorBounds = valueBounds.withSizeKeepingCentre(80, 16);
        g.setColour(juce::Colour(0xffff6600).withAlpha(0.2f));
        g.fillRoundedRectangle(indicatorBounds.toFloat(), 3.0f);
        g.setColour(juce::Colour(0xffff9933));
        g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
        g.drawText("CORRECTING", indicatorBounds, juce::Justification::centred);
    }
}

void HaasAlignDelayEditor::drawPowerButton(juce::Graphics& g, juce::Rectangle<int> bounds,
                                            juce::Colour accentColor, bool isOn)
{
    // Outer ring
    g.setColour(isOn ? accentColor : juce::Colour(0xff444444));
    g.drawEllipse(bounds.toFloat(), 2.0f);

    // Inner fill when on
    if (isOn)
    {
        g.setColour(accentColor.withAlpha(0.2f));
        g.fillEllipse(bounds.reduced(3).toFloat());
    }

    // Power icon
    float cx = static_cast<float>(bounds.getCentreX());
    float cy = static_cast<float>(bounds.getCentreY());
    float radius = bounds.getWidth() / 2.0f - 6;

    g.setColour(isOn ? accentColor : juce::Colour(0xff444444));

    // Arc
    juce::Path arc;
    arc.addCentredArc(cx, cy, radius, radius, 0,
                      juce::MathConstants<float>::pi * 0.35f,
                      juce::MathConstants<float>::pi * 1.65f, true);
    g.strokePath(arc, juce::PathStrokeType(1.5f));

    // Top line
    g.drawLine(cx, cy - radius - 1, cx, cy - 2, 1.5f);
}

void HaasAlignDelayEditor::drawWaveIcon(juce::Graphics& g, juce::Rectangle<int> bounds,
                                         juce::Colour color, bool isActive)
{
    if (!isActive)
    {
        g.setColour(color.withAlpha(0.3f));
    }
    else
    {
        g.setColour(color);
    }

    float cx = static_cast<float>(bounds.getCentreX());
    float cy = static_cast<float>(bounds.getCentreY());
    float width = static_cast<float>(bounds.getWidth());

    // First wave (offset)
    juce::Path wave1;
    float y1Offset = isActive ? std::sin(waveAnimPhase) * 3 : 0;
    wave1.startNewSubPath(bounds.getX() + 5.0f, cy + y1Offset);
    for (float x = 5; x < width - 5; x += 2)
    {
        float y = cy + std::sin((x / width) * juce::MathConstants<float>::twoPi * 2) * 10 + y1Offset;
        wave1.lineTo(bounds.getX() + x, y);
    }
    g.setColour(color.withAlpha(isActive ? 0.5f : 0.2f));
    g.strokePath(wave1, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));

    // Second wave (main)
    juce::Path wave2;
    float y2Offset = isActive ? std::sin(waveAnimPhase + 0.5f) * 3 + 5 : 5;
    wave2.startNewSubPath(bounds.getX() + 10.0f, cy + y2Offset);
    for (float x = 10; x < width - 10; x += 2)
    {
        float y = cy + std::sin((x / width) * juce::MathConstants<float>::twoPi * 2) * 8 + y2Offset;
        wave2.lineTo(bounds.getX() + x, y);
    }
    g.setColour(color.withAlpha(isActive ? 1.0f : 0.3f));
    g.strokePath(wave2, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved));

    // Glow
    if (isActive)
    {
        g.setColour(color.withAlpha(0.2f));
        g.strokePath(wave2, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved));
    }
}

void HaasAlignDelayEditor::drawExpandIcon(juce::Graphics& g, juce::Rectangle<int> bounds,
                                           juce::Colour color, bool isActive)
{
    float cx = static_cast<float>(bounds.getCentreX());
    float cy = static_cast<float>(bounds.getCentreY());
    float offset = isActive ? std::sin(waveAnimPhase * 0.5f) * 2 : 0;

    g.setColour(color.withAlpha(isActive ? 1.0f : 0.3f));

    // Expanding arrows
    auto drawArrow = [&](float startX, float startY, float endX, float endY, float arrowSize)
    {
        g.drawLine(startX, startY, endX, endY, 2.0f);
        float angle = std::atan2(endY - startY, endX - startX);
        float ax1 = endX - arrowSize * std::cos(angle - 0.5f);
        float ay1 = endY - arrowSize * std::sin(angle - 0.5f);
        float ax2 = endX - arrowSize * std::cos(angle + 0.5f);
        float ay2 = endY - arrowSize * std::sin(angle + 0.5f);
        g.drawLine(endX, endY, ax1, ay1, 2.0f);
        g.drawLine(endX, endY, ax2, ay2, 2.0f);
    };

    // Top left arrow
    drawArrow(cx, cy, cx - 15 - offset, cy - 10, 6);
    // Top right arrow
    drawArrow(cx, cy, cx + 15 + offset, cy - 10, 6);
    // Bottom left arrow
    drawArrow(cx, cy, cx - 15 - offset, cy + 10, 6);
    // Bottom right arrow
    drawArrow(cx, cy, cx + 15 + offset, cy + 10, 6);

    // Center dot
    g.fillEllipse(cx - 4, cy - 4, 8, 8);

    // Glow
    if (isActive)
    {
        g.setColour(color.withAlpha(0.3f));
        g.fillEllipse(cx - 8, cy - 8, 16, 16);
    }
}

void HaasAlignDelayEditor::drawPhaseIcon(juce::Graphics& g, juce::Rectangle<int> bounds,
                                          juce::Colour color, bool isActive, bool isCorrecting)
{
    float cx = static_cast<float>(bounds.getCentreX());
    float cy = static_cast<float>(bounds.getCentreY());
    float radius = bounds.getWidth() * 0.35f;

    juce::Colour drawColor = isCorrecting ? juce::Colour(0xffff6600) : color;
    g.setColour(drawColor.withAlpha(isActive ? 0.3f : 0.1f));
    g.drawEllipse(cx - 20, cy - 20, 40, 40, 1.5f);
    g.drawEllipse(cx - 12, cy - 12, 24, 24, 1.5f);

    // Two sine waves (phase correlation symbol)
    float scaleY = isCorrecting ? (0.8f + std::sin(waveAnimPhase * 2) * 0.2f) : 1.0f;

    juce::Path wave1;
    wave1.startNewSubPath(cx - 15, cy);
    for (float x = -15; x <= 15; x += 1)
    {
        float y = std::sin((x + 15) / 30 * juce::MathConstants<float>::twoPi) * 8 * scaleY;
        wave1.lineTo(cx + x, cy + y);
    }
    g.setColour(drawColor.withAlpha(isActive ? 1.0f : 0.3f));
    g.strokePath(wave1, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));

    juce::Path wave2;
    wave2.startNewSubPath(cx - 15, cy);
    for (float x = -15; x <= 15; x += 1)
    {
        float y = std::sin((x + 15) / 30 * juce::MathConstants<float>::twoPi + juce::MathConstants<float>::pi) * 8 * scaleY;
        wave2.lineTo(cx + x, cy + y);
    }
    g.setColour(drawColor.withAlpha(isActive ? 0.6f : 0.2f));
    g.strokePath(wave2, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));

    // AUTO text when correcting
    if (isCorrecting)
    {
        g.setColour(juce::Colour(0xffff6600));
        g.setFont(juce::FontOptions(8.0f).withStyle("Bold"));
        g.drawText("AUTO", bounds.removeFromBottom(12), juce::Justification::centred);
    }

    // Glow
    if (isActive)
    {
        g.setColour(drawColor.withAlpha(0.15f));
        g.fillEllipse(cx - 25, cy - 25, 50, 50);
    }
}

void HaasAlignDelayEditor::drawSpeakerIcon(juce::Graphics& g, juce::Rectangle<int> bounds,
                                            juce::Colour color, bool isActive)
{
    g.setColour(color.withAlpha(isActive ? 0.8f : 0.3f));

    float left = bounds.getX() + 12.0f;
    float cy = static_cast<float>(bounds.getCentreY());
    float height = bounds.getHeight() * 0.5f;

    // Speaker body
    juce::Path speaker;
    speaker.startNewSubPath(left, cy - height * 0.15f);
    speaker.lineTo(left + 12, cy - height * 0.15f);
    speaker.lineTo(left + 20, cy - height * 0.35f);
    speaker.lineTo(left + 20, cy + height * 0.35f);
    speaker.lineTo(left + 12, cy + height * 0.15f);
    speaker.lineTo(left, cy + height * 0.15f);
    speaker.closeSubPath();
    g.fillPath(speaker);

    // Sound waves
    if (isActive)
    {
        float waveX = left + 25;
        for (int i = 0; i < 3; ++i)
        {
            float waveRadius = (static_cast<float>(i) + 1.0f) * 5;
            float alpha = isActive ? (1.0f - i * 0.25f) * (0.5f + std::sin(waveAnimPhase + i * 0.5f) * 0.5f) : 0.2f;

            juce::Path arc;
            arc.addCentredArc(waveX, cy, waveRadius, waveRadius * 1.5f, 0,
                              -juce::MathConstants<float>::pi * 0.4f,
                              juce::MathConstants<float>::pi * 0.4f, true);
            g.setColour(color.withAlpha(alpha));
            g.strokePath(arc, juce::PathStrokeType(1.5f));
        }
    }

    // Glow
    if (isActive)
    {
        g.setColour(color.withAlpha(0.15f));
        g.fillEllipse(bounds.getCentreX() - 20.0f, cy - 20, 40, 40);
    }
}
