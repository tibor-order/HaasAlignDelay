/*
  ==============================================================================

    EditorLayout.h
    Layout management for the plugin editor

    Provides testable layout calculations separate from the actual editor.
    The PluginEditor uses this class for positioning components.

    Layout Structure:
    ┌──────────────────────────────────────────────────────────┐
    │ HEADER (48px)                                            │
    ├────┬─────────┬─────────┬─────────┬─────────┬────────────┤
    │ IN │  DELAY  │  WIDTH  │  PHASE  │ OUTPUT  │    OUT     │
    │ 50 │  flex   │  flex   │  flex   │  flex   │    50      │
    ├────┴─────────┴─────────┴─────────┴─────────┴────────────┤
    │ FOOTER (40px)                                            │
    └──────────────────────────────────────────────────────────┘

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "HeaderComponent.h"
#include "FooterComponent.h"
#include "MeterStrip.h"
#include "DelayModule.h"
#include "WidthModule.h"
#include "PhaseModule.h"
#include "OutputModule.h"

/**
 * @brief Layout manager for the plugin editor
 *
 * Encapsulates layout calculations for testability.
 */
class EditorLayout : public juce::Component
{
public:
    //==============================================================================
    // Constants
    //==============================================================================

    static constexpr int DEFAULT_WIDTH = 1000;
    static constexpr int DEFAULT_HEIGHT = 625;
    static constexpr int HEADER_HEIGHT = 48;
    static constexpr int FOOTER_HEIGHT = 40;
    static constexpr int METER_WIDTH = 50;
    static constexpr int MODULE_PADDING = 15;
    static constexpr int METER_REFRESH_RATE_HZ = 30;

    //==============================================================================
    // Static Methods
    //==============================================================================

    static int getDefaultWidth() { return DEFAULT_WIDTH; }
    static int getDefaultHeight() { return DEFAULT_HEIGHT; }

    static juce::String getLookAndFeelName() { return "ReOrderLookAndFeel"; }

    static int getMeterRefreshRateHz() { return METER_REFRESH_RATE_HZ; }

    static std::vector<juce::String> getRequiredParameterIds()
    {
        return {
            // Delay module
            "delayLeft",
            "delayRight",
            "delayLink",
            "delayBypass",
            // Width module
            "width",
            "widthLowCut",
            "phaseLeft",
            "phaseRight",
            "widthBypass",
            // Phase module
            "phaseThreshold",
            "correctionSpeed",
            "autoPhaseCorrection",
            "phaseBypass",
            // Output module
            "outputGain",
            "mix",
            "outputBypass",
            // Global
            "bypass"
        };
    }

    //==============================================================================
    // Constructor/Destructor
    //==============================================================================

    EditorLayout()
    {
        // Create components
        header = std::make_unique<HeaderComponent>();
        footer = std::make_unique<FooterComponent>();
        inputMeter = std::make_unique<MeterStrip>(MeterStrip::Type::Input);
        outputMeter = std::make_unique<MeterStrip>(MeterStrip::Type::Output);
        delayModule = std::make_unique<DelayModule>();
        widthModule = std::make_unique<WidthModule>();
        phaseModule = std::make_unique<PhaseModule>();
        outputModule = std::make_unique<OutputModule>();

        // Add all as children
        addAndMakeVisible(header.get());
        addAndMakeVisible(footer.get());
        addAndMakeVisible(inputMeter.get());
        addAndMakeVisible(outputMeter.get());
        addAndMakeVisible(delayModule.get());
        addAndMakeVisible(widthModule.get());
        addAndMakeVisible(phaseModule.get());
        addAndMakeVisible(outputModule.get());
    }

    ~EditorLayout() override = default;

    //==============================================================================
    // Component Queries
    //==============================================================================

    bool hasHeader() const { return header != nullptr; }
    bool hasFooter() const { return footer != nullptr; }
    bool hasInputMeter() const { return inputMeter != nullptr; }
    bool hasOutputMeter() const { return outputMeter != nullptr; }
    bool hasDelayModule() const { return delayModule != nullptr; }
    bool hasWidthModule() const { return widthModule != nullptr; }
    bool hasPhaseModule() const { return phaseModule != nullptr; }
    bool hasOutputModule() const { return outputModule != nullptr; }

    //==============================================================================
    // Component Accessors
    //==============================================================================

    HeaderComponent* getHeader() { return header.get(); }
    FooterComponent* getFooter() { return footer.get(); }
    MeterStrip* getInputMeter() { return inputMeter.get(); }
    MeterStrip* getOutputMeter() { return outputMeter.get(); }
    DelayModule* getDelayModule() { return delayModule.get(); }
    WidthModule* getWidthModule() { return widthModule.get(); }
    PhaseModule* getPhaseModule() { return phaseModule.get(); }
    OutputModule* getOutputModule() { return outputModule.get(); }

    //==============================================================================
    // Bounds Accessors (for testing layout positions)
    //==============================================================================

    juce::Rectangle<int> getHeaderBounds() const { return header->getBounds(); }
    juce::Rectangle<int> getFooterBounds() const { return footer->getBounds(); }
    juce::Rectangle<int> getInputMeterBounds() const { return inputMeter->getBounds(); }
    juce::Rectangle<int> getOutputMeterBounds() const { return outputMeter->getBounds(); }
    juce::Rectangle<int> getDelayModuleBounds() const { return delayModule->getBounds(); }
    juce::Rectangle<int> getWidthModuleBounds() const { return widthModule->getBounds(); }
    juce::Rectangle<int> getPhaseModuleBounds() const { return phaseModule->getBounds(); }
    juce::Rectangle<int> getOutputModuleBounds() const { return outputModule->getBounds(); }

    //==============================================================================
    // Component Overrides
    //==============================================================================

    void paint(juce::Graphics& g) override
    {
        g.fillAll(ReOrderColors::bgDark);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        // Header at top
        header->setBounds(bounds.removeFromTop(HEADER_HEIGHT));

        // Footer at bottom
        footer->setBounds(bounds.removeFromBottom(FOOTER_HEIGHT));

        // Content area between header and footer
        auto contentArea = bounds.reduced(MODULE_PADDING);

        // Input meter on left
        inputMeter->setBounds(contentArea.removeFromLeft(METER_WIDTH));
        contentArea.removeFromLeft(MODULE_PADDING);

        // Output meter on right
        outputMeter->setBounds(contentArea.removeFromRight(METER_WIDTH));
        contentArea.removeFromRight(MODULE_PADDING);

        // Remaining space for 4 modules
        int moduleWidth = (contentArea.getWidth() - MODULE_PADDING * 3) / 4;
        int moduleHeight = contentArea.getHeight();

        // Delay module
        delayModule->setBounds(contentArea.removeFromLeft(moduleWidth));
        contentArea.removeFromLeft(MODULE_PADDING);

        // Width module
        widthModule->setBounds(contentArea.removeFromLeft(moduleWidth));
        contentArea.removeFromLeft(MODULE_PADDING);

        // Phase module
        phaseModule->setBounds(contentArea.removeFromLeft(moduleWidth));
        contentArea.removeFromLeft(MODULE_PADDING);

        // Output module (remaining space)
        outputModule->setBounds(contentArea);
    }

private:
    //==============================================================================
    // Members
    //==============================================================================

    std::unique_ptr<HeaderComponent> header;
    std::unique_ptr<FooterComponent> footer;
    std::unique_ptr<MeterStrip> inputMeter;
    std::unique_ptr<MeterStrip> outputMeter;
    std::unique_ptr<DelayModule> delayModule;
    std::unique_ptr<WidthModule> widthModule;
    std::unique_ptr<PhaseModule> phaseModule;
    std::unique_ptr<OutputModule> outputModule;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditorLayout)
};
