#pragma once

#include <JuceHeader.h>
#include "DSP/HaasProcessor.h"

/**
 * @brief HaasAlignDelayProcessor - Audio Processing Engine
 *
 * Main audio processor class that wraps the DSP::HaasProcessor
 * and handles JUCE plugin integration (parameters, state, etc.)
 */
class HaasAlignDelayProcessor : public juce::AudioProcessor
{
public:
    HaasAlignDelayProcessor();
    ~HaasAlignDelayProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Parameter access
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Metering - delegates to DSP processor
    float getInputLevel() const { return dspProcessor.getMetering().inputLevel; }
    float getOutputLevel() const { return dspProcessor.getMetering().outputLevel; }
    float getCorrelation() const { return dspProcessor.getMetering().correlation; }

    // Utility - delegates to DSP processor
    float msToSamples(float ms) const { return dspProcessor.msToSamples(ms); }
    double getCurrentSampleRate() const { return dspProcessor.getSampleRate(); }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateDSPParameters();

    juce::AudioProcessorValueTreeState apvts;
    DSP::HaasProcessor dspProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HaasAlignDelayProcessor)
};
