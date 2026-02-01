cmake_minimum_required(VERSION 3.22)
project(TR-8655 VERSION 1.0.0)

set(JUCE_PATH "C:/Users/hjbq1/OneDrive/Desktop/VST programming/JUCE")
add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/JUCE)

# Binary data for embedded WAV samples
juce_add_binary_data(TR8655_SampleData
    HEADER_NAME "BinaryData.h"
    NAMESPACE BinaryData
    SOURCES
        "808 Sounds/808/DEEP_SUB.wav"
        "808 Sounds/808/CLOUD.wav"
        "808 Sounds/808/DIGI.wav"
        "808 Sounds/808/HOODTRAP.wav"
        "808 Sounds/808/JERK.wav"
        "808 Sounds/808/MINK.wav"
        "808 Sounds/808/PLUGG.wav"
        "808 Sounds/808/RAGE.wav"
        "808 Sounds/Bass/808_MAFIA.wav"
        "808 Sounds/Bass/REESE.wav"
        "808 Sounds/Bass/SYNTH.wav"
        "808 Sounds/Bass/ZAY.wav"
        "TR8655-Assets/png/knob-complete.png"
        "TR8655-Assets/png/visualizer-sphere.png"
        "TR8655-Assets/png/logo-header.png"
        "TR8655-Assets/png/engage-on.png"
        "TR8655-Assets/png/engage-off.png"
        "TR8655-Assets/png/meter-frame.png"
        "TR8655-Assets/png/preset-box.png"
)

juce_add_plugin(TR-8655
    COMPANY_NAME "Oasis Creative Labs"
    PLUGIN_MANUFACTURER_CODE Oasi
    PLUGIN_CODE Tr86
    FORMATS VST3 Standalone
    PRODUCT_NAME "TR-8655"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(TR-8655
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
        src/SamplerEngine/SamplerEngine.cpp
        src/Components/CustomKnob.cpp
        src/Components/VUMeter.cpp
        src/Components/Visualizer.cpp
        src/Components/PresetSelector.cpp
)

target_include_directories(TR-8655
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(TR-8655
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(TR-8655
    PRIVATE
        TR8655_SampleData
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
