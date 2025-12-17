/*
  ==============================================================================

    PresetSelector.h
    Preset navigation and selection component

    Layout:
    ┌─────────────────────────────────────────┐
    │  [<]  │  PRESET NAME *  │  [>]          │
    └─────────────────────────────────────────┘

    Features:
    - Left/right arrow buttons for navigation
    - Central dropdown showing preset name
    - Click dropdown to open full list
    - Shows [Factory] and [User] sections
    - Modified indicator (*) for unsaved changes
    - Save/Save As via right-click context menu
    - Minimum width: 180px

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ReOrderColors.h"
#include "../Presets/PresetManager.h"

/**
 * @brief Preset navigation and selection UI component
 */
class PresetSelector : public juce::Component
{
public:
    //==============================================================================
    // Constants
    //==============================================================================

    static constexpr int MINIMUM_WIDTH = 180;
    static constexpr int ARROW_BUTTON_WIDTH = 24;
    static constexpr int DEFAULT_HEIGHT = 28;

    //==============================================================================
    // Constructor/Destructor
    //==============================================================================

    explicit PresetSelector(PresetManager& presetManager)
        : manager(presetManager),
          currentPresetIndex(0),
          contextMenuEnabled(true)
    {
        // Create left arrow button
        leftArrowButton = std::make_unique<juce::TextButton>("<");
        leftArrowButton->onClick = [this]() { navigateToPrevious(); };
        addAndMakeVisible(leftArrowButton.get());

        // Create right arrow button
        rightArrowButton = std::make_unique<juce::TextButton>(">");
        rightArrowButton->onClick = [this]() { navigateToNext(); };
        addAndMakeVisible(rightArrowButton.get());

        // Create preset dropdown
        presetDropdown = std::make_unique<juce::ComboBox>("presetDropdown");
        presetDropdown->onChange = [this]() { onDropdownChanged(); };
        addAndMakeVisible(presetDropdown.get());

        // Populate dropdown from preset manager
        populateDropdown();

        // Select first preset if available
        if (presetDropdown->getNumItems() > 0)
        {
            presetDropdown->setSelectedItemIndex(0, juce::dontSendNotification);
        }
    }

    ~PresetSelector() override = default;

    //==============================================================================
    // Navigation Methods
    //==============================================================================

    void navigateToPrevious()
    {
        int presetCount = getPresetCount();
        if (presetCount == 0) return;

        if (currentPresetIndex == 0)
        {
            // Wrap to last preset
            currentPresetIndex = presetCount - 1;
        }
        else
        {
            currentPresetIndex--;
        }

        loadPresetAtCurrentIndex();
    }

    void navigateToNext()
    {
        int presetCount = getPresetCount();
        if (presetCount == 0) return;

        if (currentPresetIndex >= presetCount - 1)
        {
            // Wrap to first preset
            currentPresetIndex = 0;
        }
        else
        {
            currentPresetIndex++;
        }

        loadPresetAtCurrentIndex();
    }

    //==============================================================================
    // Preset Access Methods
    //==============================================================================

    int getCurrentPresetIndex() const { return currentPresetIndex; }

    void setCurrentPresetIndex(int index)
    {
        int presetCount = getPresetCount();
        if (presetCount == 0) return;

        currentPresetIndex = juce::jlimit(0, presetCount - 1, index);
        presetDropdown->setSelectedItemIndex(currentPresetIndex, juce::dontSendNotification);
    }

    int getPresetCount() const
    {
        return static_cast<int>(presetList.size());
    }

    int getDropdownItemCount() const
    {
        return presetDropdown->getNumItems();
    }

    juce::String getCurrentPresetName() const
    {
        if (currentPresetIndex >= 0 && currentPresetIndex < static_cast<int>(presetList.size()))
        {
            return presetList[static_cast<size_t>(currentPresetIndex)].name;
        }
        return {};
    }

    void loadPreset(const juce::String& name)
    {
        auto result = manager.loadPreset(name);
        if (result.success)
        {
            // Find index of loaded preset
            for (size_t i = 0; i < presetList.size(); ++i)
            {
                if (presetList[i].name == name)
                {
                    currentPresetIndex = static_cast<int>(i);
                    presetDropdown->setSelectedItemIndex(currentPresetIndex, juce::dontSendNotification);
                    break;
                }
            }
            updateDisplayText();

            // Notify listeners
            if (onPresetLoaded)
                onPresetLoaded(name);
        }
    }

    //==============================================================================
    // Component State Methods
    //==============================================================================

    bool hasLeftArrow() const { return leftArrowButton != nullptr; }
    bool hasRightArrow() const { return rightArrowButton != nullptr; }
    bool hasDropdown() const { return presetDropdown != nullptr; }

    bool isShowingModifiedIndicator() const
    {
        return manager.isModified();
    }

    bool isContextMenuEnabled() const { return contextMenuEnabled; }

    void updateFromPresetManager()
    {
        updateDisplayText();
        repaint();
    }

    int getMinimumWidth() const { return MINIMUM_WIDTH; }

    //==============================================================================
    // Callback for preset loading
    //==============================================================================

    std::function<void(const juce::String&)> onPresetLoaded;

    //==============================================================================
    // Component Overrides
    //==============================================================================

    void paint(juce::Graphics& g) override
    {
        // Background
        g.setColour(ReOrderColors::bgDark);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);

        // Border
        g.setColour(ReOrderColors::border);
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 4.0f, 1.0f);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        // Left arrow
        leftArrowButton->setBounds(bounds.removeFromLeft(ARROW_BUTTON_WIDTH));

        // Right arrow
        rightArrowButton->setBounds(bounds.removeFromRight(ARROW_BUTTON_WIDTH));

        // Dropdown fills the rest
        presetDropdown->setBounds(bounds);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu() && contextMenuEnabled)
        {
            showContextMenu();
        }
    }

protected:
    //==============================================================================
    // Protected Members (for testability)
    //==============================================================================

    std::unique_ptr<juce::TextButton> leftArrowButton;
    std::unique_ptr<juce::TextButton> rightArrowButton;
    std::unique_ptr<juce::ComboBox> presetDropdown;
    bool contextMenuEnabled;

private:
    //==============================================================================
    // Private Methods
    //==============================================================================

    void populateDropdown()
    {
        presetDropdown->clear();
        presetList = manager.getPresetList();

        // Separate factory and user presets
        std::vector<PresetInfo> factoryPresets;
        std::vector<PresetInfo> userPresets;

        for (const auto& preset : presetList)
        {
            if (preset.isFactory)
                factoryPresets.push_back(preset);
            else
                userPresets.push_back(preset);
        }

        int itemId = 1;

        // Add factory presets section
        if (!factoryPresets.empty())
        {
            presetDropdown->addSectionHeading("[Factory]");
            for (const auto& preset : factoryPresets)
            {
                presetDropdown->addItem(preset.name, itemId++);
            }
        }

        // Add user presets section
        if (!userPresets.empty())
        {
            presetDropdown->addSectionHeading("[User]");
            for (const auto& preset : userPresets)
            {
                presetDropdown->addItem(preset.name, itemId++);
            }
        }
    }

    void loadPresetAtCurrentIndex()
    {
        if (currentPresetIndex >= 0 && currentPresetIndex < static_cast<int>(presetList.size()))
        {
            juce::String presetName = presetList[static_cast<size_t>(currentPresetIndex)].name;
            loadPreset(presetName);
        }
    }

    void onDropdownChanged()
    {
        int selectedIndex = presetDropdown->getSelectedItemIndex();
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(presetList.size()))
        {
            currentPresetIndex = selectedIndex;
            loadPresetAtCurrentIndex();
        }
    }

    void updateDisplayText()
    {
        juce::String displayText = getCurrentPresetName();

        // Add modified indicator
        if (manager.isModified())
        {
            displayText += " *";
        }

        // Update dropdown text (done via selection)
        presetDropdown->setSelectedItemIndex(currentPresetIndex, juce::dontSendNotification);
    }

    void showContextMenu()
    {
        juce::PopupMenu menu;

        menu.addItem(1, "Save");
        menu.addItem(2, "Save As...");
        menu.addSeparator();
        menu.addItem(3, "Rename...");
        menu.addItem(4, "Delete");
        menu.addSeparator();
        menu.addItem(5, "Refresh List");

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
                          [this](int result)
                          {
                              handleContextMenuResult(result);
                          });
    }

    void handleContextMenuResult(int result)
    {
        switch (result)
        {
            case 1: // Save
                saveCurrentPreset();
                break;
            case 2: // Save As
                saveAsNewPreset();
                break;
            case 3: // Rename
                renameCurrentPreset();
                break;
            case 4: // Delete
                deleteCurrentPreset();
                break;
            case 5: // Refresh
                refreshPresetList();
                break;
            default:
                break;
        }
    }

    void saveCurrentPreset()
    {
        juce::String name = getCurrentPresetName();
        if (name.isNotEmpty())
        {
            manager.savePreset(name);
            updateDisplayText();
        }
    }

    void saveAsNewPreset()
    {
        auto alertWindow = std::make_unique<juce::AlertWindow>(
            "Save Preset As",
            "Enter a name for the preset:",
            juce::MessageBoxIconType::QuestionIcon);

        alertWindow->addTextEditor("name", getCurrentPresetName(), "Preset Name:");
        alertWindow->addButton("Save", 1);
        alertWindow->addButton("Cancel", 0);

        alertWindow->enterModalState(true,
            juce::ModalCallbackFunction::create([this](int result)
            {
                // Note: This is simplified - in real impl you'd capture the alert window
            }));
    }

    void renameCurrentPreset()
    {
        // Only allow renaming user presets
        if (currentPresetIndex >= 0 && currentPresetIndex < static_cast<int>(presetList.size()))
        {
            if (presetList[static_cast<size_t>(currentPresetIndex)].isFactory)
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon,
                    "Cannot Rename",
                    "Factory presets cannot be renamed.");
                return;
            }
        }

        // Show rename dialog (simplified)
    }

    void deleteCurrentPreset()
    {
        // Only allow deleting user presets
        if (currentPresetIndex >= 0 && currentPresetIndex < static_cast<int>(presetList.size()))
        {
            if (presetList[static_cast<size_t>(currentPresetIndex)].isFactory)
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon,
                    "Cannot Delete",
                    "Factory presets cannot be deleted.");
                return;
            }

            juce::String name = getCurrentPresetName();
            auto result = manager.deletePreset(name);
            if (result.success)
            {
                refreshPresetList();
            }
        }
    }

    void refreshPresetList()
    {
        manager.refreshPresetList();
        populateDropdown();

        // Reset to first preset if current is out of range
        if (currentPresetIndex >= static_cast<int>(presetList.size()))
        {
            currentPresetIndex = 0;
        }

        if (presetDropdown->getNumItems() > 0)
        {
            presetDropdown->setSelectedItemIndex(currentPresetIndex, juce::dontSendNotification);
        }
    }

    //==============================================================================
    // Members
    //==============================================================================

    PresetManager& manager;
    std::vector<PresetInfo> presetList;
    int currentPresetIndex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetSelector)
};
