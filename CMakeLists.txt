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
    BLUETOOTH_PERMISSION_ENABLED FALSE
    MICROPHONE_PERMISSION_ENABLED FALSE
    PLIST_TO_MERGE "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\"><plist version=\"1.0\"><dict><key>NSBluetoothAlwaysUsageDescription</key><string>This app may use Bluetooth for MIDI device connectivity.</string><key>NSMicrophoneUsageDescription</key><string>This app processes audio input.</string></dict></plist>"
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

#==============================================================================
# Test Target - Catch2 Unit Tests
#==============================================================================

enable_testing()

# Create a static library for testable source files (header-only for now)
add_library(HaasAlignDelay_TestableLib INTERFACE)
target_include_directories(HaasAlignDelay_TestableLib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)
target_link_libraries(HaasAlignDelay_TestableLib INTERFACE
    juce::juce_core
    juce::juce_graphics
)

# Test executable
add_executable(HaasAlignDelay_Tests
    Tests/catch2/catch_amalgamated.cpp
    Tests/TestMain.cpp
    Tests/UI/ReOrderColorsTest.cpp
    Tests/UI/VerticalLevelMeterTest.cpp
    Tests/UI/PhaseCorrelationMeterTest.cpp
    Tests/UI/ModuleIconsTest.cpp
    Tests/UI/ReOrderLookAndFeelTest.cpp
    Tests/UI/LinearSliderLookAndFeelTest.cpp
    Tests/UI/ButtonLookAndFeelTest.cpp
    Tests/DSP/OutputGainTest.cpp
    Tests/DSP/WidthLowCutTest.cpp
    Tests/DSP/CorrectionSpeedTest.cpp
    Tests/DSP/ModuleBypassTest.cpp
    Tests/DSP/DelayLinkTest.cpp
    Tests/PluginProcessorParamTest.cpp
    # Include PluginProcessor source for parameter tests
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/UI/VoxProLookAndFeel.cpp
)

target_include_directories(HaasAlignDelay_Tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Tests
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_BINARY_DIR}/HaasAlignDelay_artefacts/JuceLibraryCode
)

target_link_libraries(HaasAlignDelay_Tests PRIVATE
    HaasAlignDelay_TestableLib
    HaasAlignDelayBinaryData
    juce::juce_core
    juce::juce_graphics
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_basics
)

target_compile_definitions(HaasAlignDelay_Tests PRIVATE
    JUCE_STANDALONE_APPLICATION=1
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    # Plugin definitions needed for PluginProcessor
    JucePlugin_Name="Haas Align Delay"
    JucePlugin_WantsMidiInput=0
    JucePlugin_ProducesMidiOutput=0
    JucePlugin_IsMidiEffect=0
    JucePlugin_IsSynth=0
)

# Register test with CTest
add_test(NAME HaasAlignDelay_Tests COMMAND HaasAlignDelay_Tests)
