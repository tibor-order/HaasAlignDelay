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

    // Delay parameters (0-50ms range for alignment)
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

    // Delay Link (couples L/R delay sliders)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"delayLink", 1},
        "Delay Link",
        false));

    // Delay Bypass
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"delayBypass", 1},
        "Delay Bypass",
        false));

    // Width (stereo width adjustment)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"width", 1},
        "Width",
        juce::NormalisableRange<float>(0.0f, 200.0f, 1.0f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Width Low Cut (high-pass filter on side channel)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"widthLowCut", 1},
        "Width Low Cut",
        juce::NormalisableRange<float>(20.0f, 500.0f, 1.0f, 0.5f),
        250.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    // Width Bypass
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"widthBypass", 1},
        "Width Bypass",
        false));

    // Mix
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mix", 1},
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Phase invert
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"phaseLeft", 1},
        "Phase Left",
        false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"phaseRight", 1},
        "Phase Right",
        false));

    // Phase Bypass
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"phaseBypass", 1},
        "Phase Bypass",
        false));

    // Correction Speed (0-100%)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"correctionSpeed", 1},
        "Correction Speed",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Bypass
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"bypass", 1},
        "Bypass",
        false));

    // Auto Phase enabled
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"autoPhase", 1},
        "Auto Phase",
        false));

    // Phase Safety mode (0=Relaxed, 1=Balanced, 2=Strict)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"phaseSafety", 1},
        "Phase Safety",
        juce::StringArray{"Relaxed", "Balanced", "Strict"},
        1)); // Default to Balanced

    // Output Gain (-12 to +12 dB)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"outputGain", 1},
        "Output Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Output Bypass
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"outputBypass", 1},
        "Output Bypass",
        false));

    return {params.begin(), params.end()};
}

void HaasAlignDelayProcessor::updateDSPParameters()
{
    DSP::HaasParameters params;

    // Delay parameters
    params.delayLeftMs = *apvts.getRawParameterValue("delayLeft");
    params.delayRightMs = *apvts.getRawParameterValue("delayRight");
    params.delayLink = *apvts.getRawParameterValue("delayLink") > 0.5f;
    params.delayBypass = *apvts.getRawParameterValue("delayBypass") > 0.5f;

    // Width parameters
    params.width = *apvts.getRawParameterValue("width");
    params.widthLowCut = *apvts.getRawParameterValue("widthLowCut");
    params.widthBypass = *apvts.getRawParameterValue("widthBypass") > 0.5f;

    // Mix
    params.mix = *apvts.getRawParameterValue("mix");

    // Phase parameters
    params.phaseInvertLeft = *apvts.getRawParameterValue("phaseLeft") > 0.5f;
    params.phaseInvertRight = *apvts.getRawParameterValue("phaseRight") > 0.5f;
    params.phaseBypass = *apvts.getRawParameterValue("phaseBypass") > 0.5f;
    params.correctionSpeed = *apvts.getRawParameterValue("correctionSpeed");
    params.autoPhaseEnabled = *apvts.getRawParameterValue("autoPhase") > 0.5f;

    // Master bypass
    params.bypass = *apvts.getRawParameterValue("bypass") > 0.5f;

    // Output parameters
    params.outputGain = *apvts.getRawParameterValue("outputGain");
    params.outputBypass = *apvts.getRawParameterValue("outputBypass") > 0.5f;

    // Convert choice index to PhaseSafetyMode
    int safetyIndex = static_cast<int>(*apvts.getRawParameterValue("phaseSafety"));
    switch (safetyIndex)
    {
        case 0: params.phaseSafety = DSP::PhaseSafetyMode::Relaxed; break;
        case 1: params.phaseSafety = DSP::PhaseSafetyMode::Balanced; break;
        case 2: params.phaseSafety = DSP::PhaseSafetyMode::Strict; break;
        default: params.phaseSafety = DSP::PhaseSafetyMode::Balanced; break;
    }

    dspProcessor.setParameters(params);
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
