#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HaasAlignDelayProcessor::HaasAlignDelayProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         ),
#endif
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

HaasAlignDelayProcessor::~HaasAlignDelayProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout HaasAlignDelayProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // === DELAY MODULE ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"delayPower", 1},
        "Delay Power",
        true));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"delayLeft", 1},
        "Delay Left",
        juce::NormalisableRange<float>(0.0f, 50.0f, 0.01f, 0.5f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"delayRight", 1},
        "Delay Right",
        juce::NormalisableRange<float>(0.0f, 50.0f, 0.01f, 0.5f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"delayLink", 1},
        "Delay Link",
        false));

    // === WIDTH MODULE ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"widthPower", 1},
        "Width Power",
        true));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"width", 1},
        "Width",
        juce::NormalisableRange<float>(100.0f, 200.0f, 1.0f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"lowCut", 1},
        "Low Cut",
        juce::NormalisableRange<float>(20.0f, 500.0f, 1.0f, 0.4f),
        20.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    // === AUTO PHASE MODULE ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"autoPhase", 1},
        "Auto Phase",
        false));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"phaseSafety", 1},
        "Phase Safety",
        juce::StringArray{"Relaxed", "Balanced", "Strict"},
        1)); // Default to Balanced

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"threshold", 1},
        "Threshold",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.3f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"speed", 1},
        "Speed",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // === OUTPUT MODULE ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"outputPower", 1},
        "Output Power",
        true));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"outputGain", 1},
        "Output Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mix", 1},
        "Dry/Wet",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // === GLOBAL ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"bypass", 1},
        "Bypass",
        false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"phaseLeft", 1},
        "Phase Left",
        false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"phaseRight", 1},
        "Phase Right",
        false));

    return {params.begin(), params.end()};
}

void HaasAlignDelayProcessor::updateDSPParameters()
{
    DSP::HaasParameters dspParams;

    // Check module power states
    bool delayOn = *apvts.getRawParameterValue("delayPower") > 0.5f;
    bool widthOn = *apvts.getRawParameterValue("widthPower") > 0.5f;
    bool outputOn = *apvts.getRawParameterValue("outputPower") > 0.5f;

    // Delay parameters (only apply if module is on)
    if (delayOn)
    {
        dspParams.delayLeftMs = *apvts.getRawParameterValue("delayLeft");
        dspParams.delayRightMs = *apvts.getRawParameterValue("delayRight");
    }
    else
    {
        dspParams.delayLeftMs = 0.0f;
        dspParams.delayRightMs = 0.0f;
    }

    // Width parameters (only apply if module is on)
    if (widthOn)
    {
        dspParams.width = *apvts.getRawParameterValue("width");
    }
    else
    {
        dspParams.width = 100.0f; // No stereo expansion when off
    }

    // Output mix (only apply if module is on)
    if (outputOn)
    {
        dspParams.mix = *apvts.getRawParameterValue("mix");
    }
    else
    {
        dspParams.mix = 0.0f; // Full dry when off
    }

    // Phase invert
    dspParams.phaseInvertLeft = *apvts.getRawParameterValue("phaseLeft") > 0.5f;
    dspParams.phaseInvertRight = *apvts.getRawParameterValue("phaseRight") > 0.5f;

    // Bypass
    dspParams.bypass = *apvts.getRawParameterValue("bypass") > 0.5f;

    // Auto phase
    dspParams.autoPhaseEnabled = *apvts.getRawParameterValue("autoPhase") > 0.5f;

    // Phase safety mode
    int safetyIndex = static_cast<int>(*apvts.getRawParameterValue("phaseSafety"));
    switch (safetyIndex)
    {
        case 0: dspParams.phaseSafety = DSP::PhaseSafetyMode::Relaxed; break;
        case 1: dspParams.phaseSafety = DSP::PhaseSafetyMode::Balanced; break;
        case 2: dspParams.phaseSafety = DSP::PhaseSafetyMode::Strict; break;
        default: dspParams.phaseSafety = DSP::PhaseSafetyMode::Balanced; break;
    }

    // Store output gain for processing
    outputGainDb = *apvts.getRawParameterValue("outputGain");

    dspProcessor.setParameters(dspParams);
}

const juce::String HaasAlignDelayProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HaasAlignDelayProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool HaasAlignDelayProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool HaasAlignDelayProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double HaasAlignDelayProcessor::getTailLengthSeconds() const
{
    return DSP::HaasProcessor::MAX_DELAY_MS * 0.001;
}

int HaasAlignDelayProcessor::getNumPrograms()
{
    return 1;
}

int HaasAlignDelayProcessor::getCurrentProgram()
{
    return 0;
}

void HaasAlignDelayProcessor::setCurrentProgram(int)
{
}

const juce::String HaasAlignDelayProcessor::getProgramName(int)
{
    return {};
}

void HaasAlignDelayProcessor::changeProgramName(int, const juce::String&)
{
}

void HaasAlignDelayProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    dspProcessor.prepare(sampleRate, samplesPerBlock);
    updateDSPParameters();
}

void HaasAlignDelayProcessor::releaseResources()
{
    dspProcessor.release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HaasAlignDelayProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void HaasAlignDelayProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                            juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Update parameters from APVTS
    updateDSPParameters();

    // Process stereo audio
    if (totalNumInputChannels >= 2)
    {
        auto* channelL = buffer.getWritePointer(0);
        auto* channelR = buffer.getWritePointer(1);
        dspProcessor.processBlock(channelL, channelR, buffer.getNumSamples());

        // Apply output gain
        if (std::abs(outputGainDb) > 0.01f)
        {
            float gainLinear = std::pow(10.0f, outputGainDb / 20.0f);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                channelL[i] *= gainLinear;
                channelR[i] *= gainLinear;
            }
        }
    }
}

bool HaasAlignDelayProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* HaasAlignDelayProcessor::createEditor()
{
    return new HaasAlignDelayEditor(*this);
}

void HaasAlignDelayProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void HaasAlignDelayProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HaasAlignDelayProcessor();
}
