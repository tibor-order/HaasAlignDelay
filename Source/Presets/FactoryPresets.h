/*
  ==============================================================================

    FactoryPresets.h
    Embedded factory presets for HaasAlignDelay

    Provides 6+ factory presets:
    1. Default - Neutral starting point
    2. Vocal Doubler - Classic Haas for vocals
    3. Wide Stereo - Maximum stereo width
    4. Subtle Spread - Gentle stereo enhancement
    5. Mono Compatible - Phase-safe width
    6. Auto Phase - Automatic phase correction enabled

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// Forward declaration
struct PresetInfo;

/**
 * @brief Embedded factory presets
 */
class FactoryPresets
{
public:
    /**
     * @brief Preset info for factory presets (local struct to avoid circular dependency)
     */
    struct FactoryPresetInfo
    {
        juce::String name;
        bool isFactory = true;
    };

    /**
     * @brief Get all factory preset names
     */
    static std::vector<FactoryPresetInfo> getAll()
    {
        std::vector<FactoryPresetInfo> presets;

        presets.push_back({ "Default", true });
        presets.push_back({ "Vocal Doubler", true });
        presets.push_back({ "Wide Stereo", true });
        presets.push_back({ "Subtle Spread", true });
        presets.push_back({ "Mono Compatible", true });
        presets.push_back({ "Auto Phase", true });

        return presets;
    }

    /**
     * @brief Get preset XML data by name
     */
    static juce::String getPresetData(const juce::String& name)
    {
        if (name == "Default")
            return getDefaultPreset();
        if (name == "Vocal Doubler")
            return getVocalDoublerPreset();
        if (name == "Wide Stereo")
            return getWideStereoPreset();
        if (name == "Subtle Spread")
            return getSubtleSpreadPreset();
        if (name == "Mono Compatible")
            return getMonoCompatiblePreset();
        if (name == "Auto Phase")
            return getAutoPhasePreset();

        return {};
    }

private:
    static juce::String getDefaultPreset()
    {
        return R"(<?xml version="1.0" encoding="UTF-8"?>
<HaasPreset version="1" name="Default">
  <Parameters>
    <delayLeft>0.0</delayLeft>
    <delayRight>0.0</delayRight>
    <delayLink>0.0</delayLink>
    <delayBypass>0.0</delayBypass>
    <width>100.0</width>
    <widthLowCut>20.0</widthLowCut>
    <phaseLeft>0.0</phaseLeft>
    <phaseRight>0.0</phaseRight>
    <widthBypass>0.0</widthBypass>
    <phaseThreshold>0.3</phaseThreshold>
    <correctionSpeed>50.0</correctionSpeed>
    <autoPhaseCorrection>0.0</autoPhaseCorrection>
    <phaseBypass>0.0</phaseBypass>
    <outputGain>0.0</outputGain>
    <mix>100.0</mix>
    <outputBypass>0.0</outputBypass>
    <bypass>0.0</bypass>
  </Parameters>
</HaasPreset>)";
    }

    static juce::String getVocalDoublerPreset()
    {
        return R"(<?xml version="1.0" encoding="UTF-8"?>
<HaasPreset version="1" name="Vocal Doubler">
  <Parameters>
    <delayLeft>0.0</delayLeft>
    <delayRight>18.0</delayRight>
    <delayLink>0.0</delayLink>
    <delayBypass>0.0</delayBypass>
    <width>120.0</width>
    <widthLowCut>80.0</widthLowCut>
    <phaseLeft>0.0</phaseLeft>
    <phaseRight>0.0</phaseRight>
    <widthBypass>0.0</widthBypass>
    <phaseThreshold>0.4</phaseThreshold>
    <correctionSpeed>60.0</correctionSpeed>
    <autoPhaseCorrection>1.0</autoPhaseCorrection>
    <phaseBypass>0.0</phaseBypass>
    <outputGain>0.0</outputGain>
    <mix>75.0</mix>
    <outputBypass>0.0</outputBypass>
    <bypass>0.0</bypass>
  </Parameters>
</HaasPreset>)";
    }

    static juce::String getWideStereoPreset()
    {
        return R"(<?xml version="1.0" encoding="UTF-8"?>
<HaasPreset version="1" name="Wide Stereo">
  <Parameters>
    <delayLeft>0.0</delayLeft>
    <delayRight>30.0</delayRight>
    <delayLink>0.0</delayLink>
    <delayBypass>0.0</delayBypass>
    <width>180.0</width>
    <widthLowCut>100.0</widthLowCut>
    <phaseLeft>0.0</phaseLeft>
    <phaseRight>0.0</phaseRight>
    <widthBypass>0.0</widthBypass>
    <phaseThreshold>0.2</phaseThreshold>
    <correctionSpeed>40.0</correctionSpeed>
    <autoPhaseCorrection>0.0</autoPhaseCorrection>
    <phaseBypass>0.0</phaseBypass>
    <outputGain>-1.0</outputGain>
    <mix>100.0</mix>
    <outputBypass>0.0</outputBypass>
    <bypass>0.0</bypass>
  </Parameters>
</HaasPreset>)";
    }

    static juce::String getSubtleSpreadPreset()
    {
        return R"(<?xml version="1.0" encoding="UTF-8"?>
<HaasPreset version="1" name="Subtle Spread">
  <Parameters>
    <delayLeft>0.0</delayLeft>
    <delayRight>8.0</delayRight>
    <delayLink>0.0</delayLink>
    <delayBypass>0.0</delayBypass>
    <width>115.0</width>
    <widthLowCut>40.0</widthLowCut>
    <phaseLeft>0.0</phaseLeft>
    <phaseRight>0.0</phaseRight>
    <widthBypass>0.0</widthBypass>
    <phaseThreshold>0.5</phaseThreshold>
    <correctionSpeed>70.0</correctionSpeed>
    <autoPhaseCorrection>1.0</autoPhaseCorrection>
    <phaseBypass>0.0</phaseBypass>
    <outputGain>0.0</outputGain>
    <mix>50.0</mix>
    <outputBypass>0.0</outputBypass>
    <bypass>0.0</bypass>
  </Parameters>
</HaasPreset>)";
    }

    static juce::String getMonoCompatiblePreset()
    {
        return R"(<?xml version="1.0" encoding="UTF-8"?>
<HaasPreset version="1" name="Mono Compatible">
  <Parameters>
    <delayLeft>0.0</delayLeft>
    <delayRight>12.0</delayRight>
    <delayLink>0.0</delayLink>
    <delayBypass>0.0</delayBypass>
    <width>100.0</width>
    <widthLowCut>150.0</widthLowCut>
    <phaseLeft>0.0</phaseLeft>
    <phaseRight>0.0</phaseRight>
    <widthBypass>0.0</widthBypass>
    <phaseThreshold>0.6</phaseThreshold>
    <correctionSpeed>80.0</correctionSpeed>
    <autoPhaseCorrection>1.0</autoPhaseCorrection>
    <phaseBypass>0.0</phaseBypass>
    <outputGain>0.0</outputGain>
    <mix>60.0</mix>
    <outputBypass>0.0</outputBypass>
    <bypass>0.0</bypass>
  </Parameters>
</HaasPreset>)";
    }

    static juce::String getAutoPhasePreset()
    {
        return R"(<?xml version="1.0" encoding="UTF-8"?>
<HaasPreset version="1" name="Auto Phase">
  <Parameters>
    <delayLeft>0.0</delayLeft>
    <delayRight>0.0</delayRight>
    <delayLink>0.0</delayLink>
    <delayBypass>0.0</delayBypass>
    <width>100.0</width>
    <widthLowCut>20.0</widthLowCut>
    <phaseLeft>0.0</phaseLeft>
    <phaseRight>0.0</phaseRight>
    <widthBypass>0.0</widthBypass>
    <phaseThreshold>0.3</phaseThreshold>
    <correctionSpeed>50.0</correctionSpeed>
    <autoPhaseCorrection>1.0</autoPhaseCorrection>
    <phaseBypass>0.0</phaseBypass>
    <outputGain>0.0</outputGain>
    <mix>100.0</mix>
    <outputBypass>0.0</outputBypass>
    <bypass>0.0</bypass>
  </Parameters>
</HaasPreset>)";
    }
};
