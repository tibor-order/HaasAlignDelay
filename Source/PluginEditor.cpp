/*
  ==============================================================================

    PluginEditor.cpp
    HAAS FX Pro - Main Editor Implementation with ReOrder Audio Design

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HaasAlignDelayEditor::HaasAlignDelayEditor(HaasAlignDelayProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Create Header
    header = std::make_unique<HeaderComponent>();
    addAndMakeVisible(header.get());

    // Connect bypass button
    header->getBypassButton()->onClick = [this]()
    {
        auto* param = processorRef.getAPVTS().getParameter("bypass");
        if (param)
        {
            bool newState = header->getBypassButton()->getToggleState();
            param->setValueNotifyingHost(newState ? 1.0f : 0.0f);
        }
    };

    // Create Footer
    footer = std::make_unique<FooterComponent>();
    addAndMakeVisible(footer.get());

    // Create Meter Strips
    inputMeterStrip = std::make_unique<MeterStrip>(MeterStrip::Type::Input);
    addAndMakeVisible(inputMeterStrip.get());

    outputMeterStrip = std::make_unique<MeterStrip>(MeterStrip::Type::Output);
    addAndMakeVisible(outputMeterStrip.get());

    // Create Modules (using default constructors)
    delayModule = std::make_unique<DelayModule>();
    addAndMakeVisible(delayModule.get());

    widthModule = std::make_unique<WidthModule>();
    addAndMakeVisible(widthModule.get());

    phaseModule = std::make_unique<PhaseModule>();
    addAndMakeVisible(phaseModule.get());

    outputModule = std::make_unique<OutputModule>();
    addAndMakeVisible(outputModule.get());

    // Connect parameter attachments
    auto& apvts = processorRef.getAPVTS();

    // Delay module attachments
    delayLeftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "delayLeft", *delayModule->getLeftDelaySlider());
    delayRightAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "delayRight", *delayModule->getRightDelaySlider());
    delayLinkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "delayLink", *delayModule->getLinkButton());
    delayBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "delayBypass", *delayModule->getPowerButton());

    // Width module attachments
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "width", *widthModule->getWidthKnob());
    widthLowCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "widthLowCut", *widthModule->getLowCutSlider());
    phaseLeftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "phaseLeft", *widthModule->getPhaseLButton());
    phaseRightAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "phaseRight", *widthModule->getPhaseRButton());
    widthBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "widthBypass", *widthModule->getPowerButton());

    // Phase module attachments
    autoPhaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "autoPhase", *phaseModule->getAutoFixButton());
    correctionSpeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "correctionSpeed", *phaseModule->getSpeedKnob());
    phaseBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "phaseBypass", *phaseModule->getPowerButton());

    // Output module attachments
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "outputGain", *outputModule->getGainKnob());
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "mix", *outputModule->getMixSlider());
    outputBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "outputBypass", *outputModule->getPowerButton());

    // Setup animation controller
    animationController.onAnimationUpdate = [this]()
    {
        // Update phase module correction state
        phaseModule->setCorrecting(processorRef.isCorrectionActive());
        repaint();
    };
    animationController.startAnimations();

    // Start meter update timer at 30Hz
    startTimerHz(30);

    // Set size
    setSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    setResizable(false, false);
}

HaasAlignDelayEditor::~HaasAlignDelayEditor()
{
    animationController.stopAnimations();
    stopTimer();
}

//==============================================================================
void HaasAlignDelayEditor::paint(juce::Graphics& g)
{
    // Fill background with darkest color
    g.fillAll(ReOrderColors::bgDark);
}

void HaasAlignDelayEditor::resized()
{
    auto bounds = getLocalBounds();

    // Header at top
    header->setBounds(bounds.removeFromTop(HEADER_HEIGHT));

    // Footer at bottom
    footer->setBounds(bounds.removeFromBottom(FOOTER_HEIGHT));

    // Add padding
    bounds = bounds.reduced(MODULE_SPACING, MODULE_SPACING);

    // Input meter strip on left
    inputMeterStrip->setBounds(bounds.removeFromLeft(METER_STRIP_WIDTH));
    bounds.removeFromLeft(MODULE_SPACING);

    // Output meter strip on right
    outputMeterStrip->setBounds(bounds.removeFromRight(METER_STRIP_WIDTH));
    bounds.removeFromRight(MODULE_SPACING);

    // Divide remaining space for 4 modules
    int moduleWidth = (bounds.getWidth() - MODULE_SPACING * 3) / 4;

    delayModule->setBounds(bounds.removeFromLeft(moduleWidth));
    bounds.removeFromLeft(MODULE_SPACING);

    widthModule->setBounds(bounds.removeFromLeft(moduleWidth));
    bounds.removeFromLeft(MODULE_SPACING);

    phaseModule->setBounds(bounds.removeFromLeft(moduleWidth));
    bounds.removeFromLeft(MODULE_SPACING);

    outputModule->setBounds(bounds);
}

void HaasAlignDelayEditor::timerCallback()
{
    // Update meters from processor
    float inputL = processorRef.getInputLevel();
    float inputR = processorRef.getInputLevel();
    inputMeterStrip->setLevels(inputL, inputR);

    float outputL = processorRef.getOutputLevel();
    float outputR = processorRef.getOutputLevel();
    outputMeterStrip->setLevels(outputL, outputR);

    // Update correction state
    bool correcting = processorRef.isCorrectionActive();
    phaseModule->setCorrecting(correcting);
    animationController.setCorrecting(correcting);

    // Update bypass button state
    auto* bypassParam = processorRef.getAPVTS().getParameter("bypass");
    if (bypassParam)
    {
        bool bypassed = bypassParam->getValue() > 0.5f;
        header->getBypassButton()->setToggleState(bypassed, juce::dontSendNotification);
    }
}
