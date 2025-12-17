/*
  ==============================================================================

    PresetManagerTest.cpp
    Tests for Preset Manager (Iteration 22)

    Tests verify:
    - Factory presets exist (at least 6)
    - Load valid preset file
    - Load invalid preset file returns error
    - Save preset creates file
    - Saved preset is valid XML
    - Saved preset includes all parameters
    - Get preset list
    - Preset list includes factory presets
    - Preset list includes user presets
    - Delete preset
    - Rename preset
    - Correct macOS preset directory
    - Preset version attribute
    - Modified indicator tracks unsaved changes

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/Presets/PresetManager.h"
#include <fstream>

using Catch::Approx;

//==============================================================================
// Helper to create temp directory for tests
//==============================================================================
class TempPresetDir
{
public:
    TempPresetDir()
    {
        tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                      .getChildFile("HaasAlignDelayTests_" + juce::String(juce::Random::getSystemRandom().nextInt()));
        tempDir.createDirectory();
    }

    ~TempPresetDir()
    {
        tempDir.deleteRecursively();
    }

    juce::File getDir() const { return tempDir; }

private:
    juce::File tempDir;
};

//==============================================================================
// T22.1 - Factory Presets Exist (at least 6)
//==============================================================================
TEST_CASE("T22.1 PresetManager_FactoryPresetsExist", "[presets]")
{
    PresetManager manager;
    auto factoryPresets = manager.getFactoryPresets();
    REQUIRE(factoryPresets.size() >= 6);
}

//==============================================================================
// T22.2 - Load Valid Preset File
//==============================================================================
TEST_CASE("T22.2 PresetManager_LoadPreset_ValidFile", "[presets]")
{
    TempPresetDir tempDir;
    PresetManager manager;
    manager.setUserPresetDirectory(tempDir.getDir());

    // Create a valid preset file
    juce::File presetFile = tempDir.getDir().getChildFile("TestPreset.preset");
    juce::String xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<HaasPreset version="1" name="TestPreset">
  <Parameters>
    <delayLeft>5.0</delayLeft>
    <delayRight>10.0</delayRight>
    <width>150.0</width>
    <mix>80.0</mix>
  </Parameters>
</HaasPreset>)";
    presetFile.replaceWithText(xmlContent);

    // Load the preset
    auto result = manager.loadPreset(presetFile);
    REQUIRE(result.success == true);

    // Verify values were loaded
    REQUIRE(manager.getParameterValue("delayLeft") == Approx(5.0));
    REQUIRE(manager.getParameterValue("delayRight") == Approx(10.0));
}

//==============================================================================
// T22.3 - Load Invalid Preset File Returns Error
//==============================================================================
TEST_CASE("T22.3 PresetManager_LoadPreset_InvalidFile", "[presets]")
{
    TempPresetDir tempDir;
    PresetManager manager;

    // Try to load non-existent file
    juce::File badFile = tempDir.getDir().getChildFile("NonExistent.preset");
    auto result = manager.loadPreset(badFile);
    REQUIRE(result.success == false);

    // Try to load invalid XML
    juce::File invalidFile = tempDir.getDir().getChildFile("Invalid.preset");
    invalidFile.replaceWithText("This is not valid XML");
    result = manager.loadPreset(invalidFile);
    REQUIRE(result.success == false);
}

//==============================================================================
// T22.4 - Save Preset Creates File
//==============================================================================
TEST_CASE("T22.4 PresetManager_SavePreset_CreatesFile", "[presets]")
{
    TempPresetDir tempDir;
    PresetManager manager;
    manager.setUserPresetDirectory(tempDir.getDir());

    juce::File presetFile = tempDir.getDir().getChildFile("NewPreset.preset");
    REQUIRE(presetFile.existsAsFile() == false);

    auto result = manager.savePreset("NewPreset");
    REQUIRE(result.success == true);
    REQUIRE(presetFile.existsAsFile() == true);
}

//==============================================================================
// T22.5 - Save Preset Valid XML
//==============================================================================
TEST_CASE("T22.5 PresetManager_SavePreset_ValidXML", "[presets]")
{
    TempPresetDir tempDir;
    PresetManager manager;
    manager.setUserPresetDirectory(tempDir.getDir());

    manager.savePreset("XMLTest");
    juce::File presetFile = tempDir.getDir().getChildFile("XMLTest.preset");

    // Parse as XML
    auto xml = juce::XmlDocument::parse(presetFile);
    REQUIRE(xml != nullptr);
    REQUIRE(xml->getTagName() == "HaasPreset");
}

//==============================================================================
// T22.6 - Save Preset All Parameters
//==============================================================================
TEST_CASE("T22.6 PresetManager_SavePreset_AllParams", "[presets]")
{
    TempPresetDir tempDir;
    PresetManager manager;
    manager.setUserPresetDirectory(tempDir.getDir());

    // Set some parameter values
    manager.setParameterValue("delayLeft", 15.0);
    manager.setParameterValue("width", 175.0);

    manager.savePreset("ParamTest");
    juce::File presetFile = tempDir.getDir().getChildFile("ParamTest.preset");

    auto xml = juce::XmlDocument::parse(presetFile);
    REQUIRE(xml != nullptr);

    auto* params = xml->getChildByName("Parameters");
    REQUIRE(params != nullptr);

    // Check essential parameters are present
    REQUIRE(params->getChildByName("delayLeft") != nullptr);
    REQUIRE(params->getChildByName("delayRight") != nullptr);
    REQUIRE(params->getChildByName("width") != nullptr);
    REQUIRE(params->getChildByName("mix") != nullptr);
}

//==============================================================================
// T22.7 - Get Preset List
//==============================================================================
TEST_CASE("T22.7 PresetManager_GetPresetList", "[presets]")
{
    PresetManager manager;
    auto presetList = manager.getPresetList();
    REQUIRE(presetList.size() > 0);
}

//==============================================================================
// T22.8 - Get Preset List Includes Factory
//==============================================================================
TEST_CASE("T22.8 PresetManager_GetPresetList_IncludesFactory", "[presets]")
{
    PresetManager manager;
    auto presetList = manager.getPresetList();

    // Check that factory presets are in the list
    bool hasFactoryPreset = false;
    for (const auto& preset : presetList)
    {
        if (preset.isFactory)
        {
            hasFactoryPreset = true;
            break;
        }
    }
    REQUIRE(hasFactoryPreset == true);
}

//==============================================================================
// T22.9 - Get Preset List Includes User
//==============================================================================
TEST_CASE("T22.9 PresetManager_GetPresetList_IncludesUser", "[presets]")
{
    TempPresetDir tempDir;
    PresetManager manager;
    manager.setUserPresetDirectory(tempDir.getDir());

    // Save a user preset
    manager.savePreset("UserTestPreset");

    // Refresh and get list
    manager.refreshPresetList();
    auto presetList = manager.getPresetList();

    // Check that user preset is in the list
    bool hasUserPreset = false;
    for (const auto& preset : presetList)
    {
        if (!preset.isFactory && preset.name == "UserTestPreset")
        {
            hasUserPreset = true;
            break;
        }
    }
    REQUIRE(hasUserPreset == true);
}

//==============================================================================
// T22.10 - Delete Preset
//==============================================================================
TEST_CASE("T22.10 PresetManager_DeletePreset", "[presets]")
{
    TempPresetDir tempDir;
    PresetManager manager;
    manager.setUserPresetDirectory(tempDir.getDir());

    // Save a preset
    manager.savePreset("ToDelete");
    juce::File presetFile = tempDir.getDir().getChildFile("ToDelete.preset");
    REQUIRE(presetFile.existsAsFile() == true);

    // Delete it
    auto result = manager.deletePreset("ToDelete");
    REQUIRE(result.success == true);
    REQUIRE(presetFile.existsAsFile() == false);
}

//==============================================================================
// T22.11 - Rename Preset
//==============================================================================
TEST_CASE("T22.11 PresetManager_RenamePreset", "[presets]")
{
    TempPresetDir tempDir;
    PresetManager manager;
    manager.setUserPresetDirectory(tempDir.getDir());

    // Save a preset
    manager.savePreset("OldName");
    juce::File oldFile = tempDir.getDir().getChildFile("OldName.preset");
    REQUIRE(oldFile.existsAsFile() == true);

    // Rename it
    auto result = manager.renamePreset("OldName", "NewName");
    REQUIRE(result.success == true);

    juce::File newFile = tempDir.getDir().getChildFile("NewName.preset");
    REQUIRE(oldFile.existsAsFile() == false);
    REQUIRE(newFile.existsAsFile() == true);
}

//==============================================================================
// T22.12 - Preset Directory macOS
//==============================================================================
TEST_CASE("T22.12 PresetManager_PresetDirectory_macOS", "[presets]")
{
    PresetManager manager;
    juce::File presetDir = manager.getDefaultUserPresetDirectory();

    // On macOS, should be in ~/Library/Audio/Presets/Reorder/HaasAlignDelay
    #if JUCE_MAC
    juce::String path = presetDir.getFullPathName();
    REQUIRE(path.contains("Library"));
    REQUIRE((path.contains("Presets") || path.contains("Audio")));
    #endif

    // Directory path should not be empty
    REQUIRE(presetDir.getFullPathName().isNotEmpty());
}

//==============================================================================
// T22.13 - Preset Version Attribute
//==============================================================================
TEST_CASE("T22.13 PresetManager_PresetVersion", "[presets]")
{
    TempPresetDir tempDir;
    PresetManager manager;
    manager.setUserPresetDirectory(tempDir.getDir());

    manager.savePreset("VersionTest");
    juce::File presetFile = tempDir.getDir().getChildFile("VersionTest.preset");

    auto xml = juce::XmlDocument::parse(presetFile);
    REQUIRE(xml != nullptr);
    REQUIRE(xml->hasAttribute("version"));

    int version = xml->getIntAttribute("version", 0);
    REQUIRE(version >= 1);
}

//==============================================================================
// T22.14 - Modified Indicator
//==============================================================================
TEST_CASE("T22.14 PresetManager_ModifiedIndicator", "[presets]")
{
    TempPresetDir tempDir;
    PresetManager manager;
    manager.setUserPresetDirectory(tempDir.getDir());

    // Initially not modified
    REQUIRE(manager.isModified() == false);

    // Change a parameter
    manager.setParameterValue("delayLeft", 25.0);

    // Should now be modified
    REQUIRE(manager.isModified() == true);

    // Save preset
    manager.savePreset("ModifiedTest");

    // Should no longer be modified
    REQUIRE(manager.isModified() == false);
}
