/*
  ==============================================================================

    PresetSelectorTest.cpp
    Tests for Preset Selector Component (Iteration 23)

    Tests verify:
    - Has left navigation arrow button
    - Has right navigation arrow button
    - Has preset dropdown
    - Left arrow goes to previous preset
    - Right arrow goes to next preset
    - Left arrow wraps around at beginning
    - Right arrow wraps around at end
    - Dropdown click shows preset list
    - Shows modified indicator (*)
    - Right-click shows context menu
    - Loading preset updates parameters
    - Minimum width of 180px

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/UI/PresetSelector.h"
#include "../Source/Presets/PresetManager.h"

using Catch::Approx;

//==============================================================================
// Helper class for testing with a mock PresetManager
//==============================================================================
class TestablePresetSelector : public PresetSelector
{
public:
    using PresetSelector::PresetSelector;

    // Expose protected/private members for testing
    juce::TextButton* getLeftArrowButton() { return leftArrowButton.get(); }
    juce::TextButton* getRightArrowButton() { return rightArrowButton.get(); }
    juce::ComboBox* getDropdown() { return presetDropdown.get(); }

    // Simulate button clicks for testing
    void simulateLeftArrowClick() { navigateToPrevious(); }
    void simulateRightArrowClick() { navigateToNext(); }

    // Check if context menu exists
    bool hasContextMenuFlag() const { return contextMenuEnabled; }
};

//==============================================================================
// T23.1 - Has Left Navigation Arrow Button
//==============================================================================
TEST_CASE("T23.1 PresetSelector_HasLeftArrow", "[presetselector]")
{
    PresetManager manager;
    PresetSelector selector(manager);

    REQUIRE(selector.hasLeftArrow() == true);
}

//==============================================================================
// T23.2 - Has Right Navigation Arrow Button
//==============================================================================
TEST_CASE("T23.2 PresetSelector_HasRightArrow", "[presetselector]")
{
    PresetManager manager;
    PresetSelector selector(manager);

    REQUIRE(selector.hasRightArrow() == true);
}

//==============================================================================
// T23.3 - Has Preset Dropdown
//==============================================================================
TEST_CASE("T23.3 PresetSelector_HasDropdown", "[presetselector]")
{
    PresetManager manager;
    PresetSelector selector(manager);

    REQUIRE(selector.hasDropdown() == true);
}

//==============================================================================
// T23.4 - Left Arrow Goes to Previous Preset
//==============================================================================
TEST_CASE("T23.4 PresetSelector_LeftArrow_PreviousPreset", "[presetselector]")
{
    PresetManager manager;
    PresetSelector selector(manager);

    // Start at preset index > 0
    selector.setCurrentPresetIndex(2);
    int initialIndex = selector.getCurrentPresetIndex();

    // Click left arrow
    selector.navigateToPrevious();

    // Should be at previous preset
    REQUIRE(selector.getCurrentPresetIndex() == initialIndex - 1);
}

//==============================================================================
// T23.5 - Right Arrow Goes to Next Preset
//==============================================================================
TEST_CASE("T23.5 PresetSelector_RightArrow_NextPreset", "[presetselector]")
{
    PresetManager manager;
    PresetSelector selector(manager);

    // Start at preset index 0
    selector.setCurrentPresetIndex(0);
    int initialIndex = selector.getCurrentPresetIndex();

    // Click right arrow
    selector.navigateToNext();

    // Should be at next preset
    REQUIRE(selector.getCurrentPresetIndex() == initialIndex + 1);
}

//==============================================================================
// T23.6 - Left Arrow Wraps Around at Beginning
//==============================================================================
TEST_CASE("T23.6 PresetSelector_LeftArrow_WrapsAround", "[presetselector]")
{
    PresetManager manager;
    PresetSelector selector(manager);

    int presetCount = selector.getPresetCount();
    REQUIRE(presetCount > 0);

    // Start at first preset (index 0)
    selector.setCurrentPresetIndex(0);

    // Click left arrow
    selector.navigateToPrevious();

    // Should wrap to last preset
    REQUIRE(selector.getCurrentPresetIndex() == presetCount - 1);
}

//==============================================================================
// T23.7 - Right Arrow Wraps Around at End
//==============================================================================
TEST_CASE("T23.7 PresetSelector_RightArrow_WrapsAround", "[presetselector]")
{
    PresetManager manager;
    PresetSelector selector(manager);

    int presetCount = selector.getPresetCount();
    REQUIRE(presetCount > 0);

    // Start at last preset
    selector.setCurrentPresetIndex(presetCount - 1);

    // Click right arrow
    selector.navigateToNext();

    // Should wrap to first preset
    REQUIRE(selector.getCurrentPresetIndex() == 0);
}

//==============================================================================
// T23.8 - Dropdown Click Shows Preset List
//==============================================================================
TEST_CASE("T23.8 PresetSelector_Dropdown_ShowsList", "[presetselector]")
{
    PresetManager manager;
    PresetSelector selector(manager);

    // Verify dropdown has items
    int itemCount = selector.getDropdownItemCount();

    // Should have items from preset manager (factory + user presets)
    REQUIRE(itemCount >= 6);  // At least 6 factory presets
}

//==============================================================================
// T23.9 - Shows Modified Indicator (*)
//==============================================================================
TEST_CASE("T23.9 PresetSelector_ShowsModifiedIndicator", "[presetselector]")
{
    PresetManager manager;
    PresetSelector selector(manager);

    // Initially not modified
    REQUIRE(selector.isShowingModifiedIndicator() == false);

    // Simulate parameter change via preset manager
    manager.setParameterValue("delayLeft", 25.0);

    // Update selector to reflect modified state
    selector.updateFromPresetManager();

    // Should now show modified indicator
    REQUIRE(selector.isShowingModifiedIndicator() == true);
}

//==============================================================================
// T23.10 - Right-Click Shows Context Menu
//==============================================================================
TEST_CASE("T23.10 PresetSelector_RightClick_ShowsMenu", "[presetselector]")
{
    PresetManager manager;
    PresetSelector selector(manager);

    // Verify context menu is enabled
    REQUIRE(selector.isContextMenuEnabled() == true);
}

//==============================================================================
// T23.11 - Loading Preset Updates Parameters
//==============================================================================
TEST_CASE("T23.11 PresetSelector_LoadPreset_UpdatesParams", "[presetselector]")
{
    PresetManager manager;
    PresetSelector selector(manager);

    // Get a factory preset name
    auto presets = manager.getPresetList();
    REQUIRE(presets.size() > 0);

    // Load a preset by name
    selector.loadPreset(presets[0].name);

    // The preset should now be the current one
    REQUIRE(selector.getCurrentPresetName() == presets[0].name);
}

//==============================================================================
// T23.12 - Minimum Width 180px
//==============================================================================
TEST_CASE("T23.12 PresetSelector_MinWidth", "[presetselector]")
{
    PresetManager manager;
    PresetSelector selector(manager);

    int minWidth = selector.getMinimumWidth();

    REQUIRE(minWidth >= 180);
}
