cmake_minimum_required(VERSION 3.22)

project(HaasAlignDelay VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find JUCE - adjust path as needed
add_subdirectory(/Users/reorderdj/JUCE ${CMAKE_BINARY_DIR}/JUCE)

juce_add_plugin(HaasAlignDelay
    COMPANY_NAME "Reorder"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Rord
    PLUGIN_CODE Haas
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Haas Align Delay"
)

juce_generate_juce_header(HaasAlignDelay)

target_sources(HaasAlignDelay
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/UI/VoxProLookAndFeel.cpp
)

# Add binary data (logo)
juce_add_binary_data(HaasAlignDelayBinaryData
    SOURCES
        Resources/logo.png
)

target_compile_definitions(HaasAlignDelay
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(HaasAlignDelay
    PRIVATE
        HaasAlignDelayBinaryData
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_basics
        juce::juce_graphics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
