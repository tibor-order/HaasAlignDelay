#pragma once

#include <JuceHeader.h>
#include "NuroColors.h"

namespace UI
{

/**
 * @brief ResizeTriangle - Corner resize indicator with size menu
 *
 * Standard plugin resize corner triangle (like Waves plugins).
 * Clicking shows a popup menu with size presets.
 */
class ResizeTriangle : public juce::Component
{
public:
    struct SizePreset
    {
        juce::String name;
        int width;
        int height;
    };

    ResizeTriangle()
    {
        setOpaque(false);
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    void setSizePresets(const std::vector<SizePreset>& presets)
    {
        sizePresets = presets;
    }

    void setCurrentPresetIndex(int index)
    {
        currentPresetIndex = index;
        repaint();
    }

    int getCurrentPresetIndex() const { return currentPresetIndex; }

    std::function<void(int)> onSizeSelected;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float size = juce::jmin(bounds.getWidth(), bounds.getHeight());

        // Determine color based on hover state
        float alpha = isMouseOver() ? 0.7f : 0.4f;
        g.setColour(juce::Colour(0xff606060).withAlpha(alpha));

        // Draw three diagonal lines forming a corner triangle
        float lineWidth = 1.5f;
        float spacing = size / 4.0f;
        float bottomRight = size - 2.0f;

        // Line 1 (shortest, top-right)
        g.drawLine(bottomRight - spacing, bottomRight,
                   bottomRight, bottomRight - spacing, lineWidth);

        // Line 2 (medium)
        g.drawLine(bottomRight - spacing * 2, bottomRight,
                   bottomRight, bottomRight - spacing * 2, lineWidth);

        // Line 3 (longest, bottom-left)
        g.drawLine(bottomRight - spacing * 3, bottomRight,
                   bottomRight, bottomRight - spacing * 3, lineWidth);
    }

    void mouseEnter(const juce::MouseEvent&) override
    {
        repaint();
    }

    void mouseExit(const juce::MouseEvent&) override
    {
        repaint();
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        if (e.mouseWasClicked() && !sizePresets.empty())
        {
            showSizeMenu();
        }
    }

private:
    void showSizeMenu()
    {
        juce::PopupMenu menu;

        for (int i = 0; i < static_cast<int>(sizePresets.size()); ++i)
        {
            const auto& preset = sizePresets[static_cast<size_t>(i)];
            juce::String itemText = preset.name + " (" +
                                    juce::String(preset.width) + " x " +
                                    juce::String(preset.height) + ")";

            bool isCurrent = (i == currentPresetIndex);

            menu.addItem(i + 1, itemText, true, isCurrent);
        }

        // Style the menu
        juce::PopupMenu::Options options;
        options = options.withTargetComponent(this)
                        .withMinimumWidth(180)
                        .withPreferredPopupDirection(juce::PopupMenu::Options::PopupDirection::upwards);

        menu.showMenuAsync(options, [this](int result)
        {
            if (result > 0 && onSizeSelected)
            {
                currentPresetIndex = result - 1;
                onSizeSelected(currentPresetIndex);
                repaint();
            }
        });
    }

    std::vector<SizePreset> sizePresets;
    int currentPresetIndex = 1;  // Default to Medium

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ResizeTriangle)
};

} // namespace UI
