/*
  ==============================================================================

    PresetManager.h
    Manages preset loading, saving, and organization

    Features:
    - Factory presets embedded in binary
    - User preset directory (platform-specific)
    - XML-based preset format
    - Modified indicator for unsaved changes

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FactoryPresets.h"

/**
 * @brief Result of a preset operation
 */
struct PresetResult
{
    bool success = false;
    juce::String errorMessage;
};

/**
 * @brief Information about a preset
 */
struct PresetInfo
{
    juce::String name;
    juce::File file;
    bool isFactory = false;
};

/**
 * @brief Manages preset loading, saving, and organization
 */
class PresetManager
{
public:
    //==============================================================================
    // Constants
    //==============================================================================

    static constexpr int PRESET_VERSION = 1;
    static constexpr const char* PRESET_EXTENSION = ".preset";
    static constexpr const char* COMPANY_NAME = "Reorder";
    static constexpr const char* PLUGIN_NAME = "HaasAlignDelay";

    //==============================================================================
    // Constructor/Destructor
    //==============================================================================

    PresetManager()
        : modified(false)
    {
        // Initialize default parameter values
        initializeDefaultParameters();

        // Set default user preset directory
        userPresetDirectory = getDefaultUserPresetDirectory();
        userPresetDirectory.createDirectory();

        // Load factory presets
        loadFactoryPresets();
    }

    ~PresetManager() = default;

    //==============================================================================
    // Factory Presets
    //==============================================================================

    std::vector<PresetInfo> getFactoryPresets() const
    {
        return factoryPresets;
    }

    //==============================================================================
    // Preset List
    //==============================================================================

    std::vector<PresetInfo> getPresetList() const
    {
        std::vector<PresetInfo> allPresets;

        // Add factory presets
        for (const auto& preset : factoryPresets)
        {
            allPresets.push_back(preset);
        }

        // Add user presets
        for (const auto& preset : userPresets)
        {
            allPresets.push_back(preset);
        }

        return allPresets;
    }

    void refreshPresetList()
    {
        userPresets.clear();

        // Scan user preset directory
        if (userPresetDirectory.isDirectory())
        {
            for (const auto& file : userPresetDirectory.findChildFiles(
                     juce::File::findFiles, false, "*" + juce::String(PRESET_EXTENSION)))
            {
                PresetInfo info;
                info.name = file.getFileNameWithoutExtension();
                info.file = file;
                info.isFactory = false;
                userPresets.push_back(info);
            }
        }
    }

    //==============================================================================
    // Load/Save
    //==============================================================================

    PresetResult loadPreset(const juce::File& file)
    {
        PresetResult result;

        if (!file.existsAsFile())
        {
            result.success = false;
            result.errorMessage = "File does not exist";
            return result;
        }

        auto xml = juce::XmlDocument::parse(file);
        if (xml == nullptr)
        {
            result.success = false;
            result.errorMessage = "Invalid XML format";
            return result;
        }

        if (xml->getTagName() != "HaasPreset")
        {
            result.success = false;
            result.errorMessage = "Not a valid HaasPreset file";
            return result;
        }

        // Load parameters
        auto* params = xml->getChildByName("Parameters");
        if (params != nullptr)
        {
            for (auto& [paramId, value] : parameterValues)
            {
                auto* paramElement = params->getChildByName(paramId);
                if (paramElement != nullptr)
                {
                    value = paramElement->getAllSubText().getDoubleValue();
                }
            }
        }

        currentPresetName = xml->getStringAttribute("name", file.getFileNameWithoutExtension());
        modified = false;
        result.success = true;
        return result;
    }

    PresetResult loadPreset(const juce::String& name)
    {
        // First check factory presets
        for (const auto& preset : factoryPresets)
        {
            if (preset.name == name)
            {
                return loadFactoryPresetByName(name);
            }
        }

        // Then check user presets
        juce::File file = userPresetDirectory.getChildFile(name + PRESET_EXTENSION);
        return loadPreset(file);
    }

    PresetResult savePreset(const juce::String& name)
    {
        PresetResult result;

        juce::File file = userPresetDirectory.getChildFile(name + PRESET_EXTENSION);

        // Create XML structure
        auto xml = std::make_unique<juce::XmlElement>("HaasPreset");
        xml->setAttribute("version", PRESET_VERSION);
        xml->setAttribute("name", name);

        // Add parameters
        auto* params = xml->createNewChildElement("Parameters");
        for (const auto& [paramId, value] : parameterValues)
        {
            auto* paramElement = params->createNewChildElement(paramId);
            paramElement->addTextElement(juce::String(value));
        }

        // Write to file
        if (!xml->writeTo(file))
        {
            result.success = false;
            result.errorMessage = "Failed to write file";
            return result;
        }

        currentPresetName = name;
        modified = false;

        // Refresh list to include new preset
        refreshPresetList();

        result.success = true;
        return result;
    }

    //==============================================================================
    // Delete/Rename
    //==============================================================================

    PresetResult deletePreset(const juce::String& name)
    {
        PresetResult result;

        juce::File file = userPresetDirectory.getChildFile(name + PRESET_EXTENSION);

        if (!file.existsAsFile())
        {
            result.success = false;
            result.errorMessage = "Preset does not exist";
            return result;
        }

        if (!file.deleteFile())
        {
            result.success = false;
            result.errorMessage = "Failed to delete file";
            return result;
        }

        refreshPresetList();
        result.success = true;
        return result;
    }

    PresetResult renamePreset(const juce::String& oldName, const juce::String& newName)
    {
        PresetResult result;

        juce::File oldFile = userPresetDirectory.getChildFile(oldName + PRESET_EXTENSION);
        juce::File newFile = userPresetDirectory.getChildFile(newName + PRESET_EXTENSION);

        if (!oldFile.existsAsFile())
        {
            result.success = false;
            result.errorMessage = "Preset does not exist";
            return result;
        }

        if (newFile.existsAsFile())
        {
            result.success = false;
            result.errorMessage = "A preset with that name already exists";
            return result;
        }

        // Load, update name, and save to new file
        auto xml = juce::XmlDocument::parse(oldFile);
        if (xml != nullptr)
        {
            xml->setAttribute("name", newName);
            xml->writeTo(newFile);
            oldFile.deleteFile();
        }
        else
        {
            // Just rename the file
            oldFile.moveFileTo(newFile);
        }

        refreshPresetList();
        result.success = true;
        return result;
    }

    //==============================================================================
    // Directory Management
    //==============================================================================

    juce::File getDefaultUserPresetDirectory() const
    {
        #if JUCE_MAC
        // Standard macOS location: ~/Library/Audio/Presets/Company/Plugin
        return juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                   .getChildFile("Library/Audio/Presets")
                   .getChildFile(COMPANY_NAME)
                   .getChildFile(PLUGIN_NAME);
        #elif JUCE_WINDOWS
        return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                   .getChildFile(COMPANY_NAME)
                   .getChildFile(PLUGIN_NAME)
                   .getChildFile("Presets");
        #else
        return juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                   .getChildFile("." + juce::String(COMPANY_NAME))
                   .getChildFile(PLUGIN_NAME)
                   .getChildFile("Presets");
        #endif
    }

    void setUserPresetDirectory(const juce::File& directory)
    {
        userPresetDirectory = directory;
        userPresetDirectory.createDirectory();
        refreshPresetList();
    }

    juce::File getUserPresetDirectory() const
    {
        return userPresetDirectory;
    }

    //==============================================================================
    // Parameter Access
    //==============================================================================

    double getParameterValue(const juce::String& paramId) const
    {
        auto it = parameterValues.find(paramId);
        if (it != parameterValues.end())
            return it->second;
        return 0.0;
    }

    void setParameterValue(const juce::String& paramId, double value)
    {
        auto it = parameterValues.find(paramId);
        if (it != parameterValues.end())
        {
            if (it->second != value)
            {
                it->second = value;
                modified = true;
            }
        }
    }

    //==============================================================================
    // Modified State
    //==============================================================================

    bool isModified() const { return modified; }
    void setModified(bool state) { modified = state; }

    juce::String getCurrentPresetName() const { return currentPresetName; }

private:
    void initializeDefaultParameters()
    {
        // Delay module
        parameterValues["delayLeft"] = 0.0;
        parameterValues["delayRight"] = 0.0;
        parameterValues["delayLink"] = 0.0;
        parameterValues["delayBypass"] = 0.0;

        // Width module
        parameterValues["width"] = 100.0;
        parameterValues["widthLowCut"] = 20.0;
        parameterValues["phaseLeft"] = 0.0;
        parameterValues["phaseRight"] = 0.0;
        parameterValues["widthBypass"] = 0.0;

        // Phase module
        parameterValues["phaseThreshold"] = 0.3;
        parameterValues["correctionSpeed"] = 50.0;
        parameterValues["autoPhaseCorrection"] = 1.0;
        parameterValues["phaseBypass"] = 0.0;

        // Output module
        parameterValues["outputGain"] = 0.0;
        parameterValues["mix"] = 100.0;
        parameterValues["outputBypass"] = 0.0;

        // Global
        parameterValues["bypass"] = 0.0;
    }

    void loadFactoryPresets()
    {
        factoryPresets.clear();
        auto factoryInfos = FactoryPresets::getAll();
        for (const auto& info : factoryInfos)
        {
            PresetInfo preset;
            preset.name = info.name;
            preset.file = juce::File();  // Factory presets have no file
            preset.isFactory = true;
            factoryPresets.push_back(preset);
        }
    }

    PresetResult loadFactoryPresetByName(const juce::String& name)
    {
        PresetResult result;
        auto data = FactoryPresets::getPresetData(name);

        if (data.isEmpty())
        {
            result.success = false;
            result.errorMessage = "Factory preset not found";
            return result;
        }

        // Parse embedded XML
        auto xml = juce::XmlDocument::parse(data);
        if (xml == nullptr)
        {
            result.success = false;
            result.errorMessage = "Invalid factory preset data";
            return result;
        }

        // Load parameters
        auto* params = xml->getChildByName("Parameters");
        if (params != nullptr)
        {
            for (auto& [paramId, value] : parameterValues)
            {
                auto* paramElement = params->getChildByName(paramId);
                if (paramElement != nullptr)
                {
                    value = paramElement->getAllSubText().getDoubleValue();
                }
            }
        }

        currentPresetName = name;
        modified = false;
        result.success = true;
        return result;
    }

    //==============================================================================
    // Members
    //==============================================================================

    juce::File userPresetDirectory;
    std::vector<PresetInfo> factoryPresets;
    std::vector<PresetInfo> userPresets;
    std::map<juce::String, double> parameterValues;
    juce::String currentPresetName;
    bool modified;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
