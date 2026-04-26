cmake_minimum_required(VERSION 3.22)

project(NimbusReverb VERSION 0.1.0)

set(JUCE_DIR "/Users/josephshively/Desktop/JUCE" CACHE PATH "Path to the local JUCE checkout")

if (NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "JUCE was not found at ${JUCE_DIR}")
endif()

add_subdirectory("${JUCE_DIR}" JUCE)

juce_add_plugin(NimbusReverb
    COMPANY_NAME "Codex"
    PLUGIN_MANUFACTURER_CODE Cdxv
    PLUGIN_CODE Nbrv
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Nimbus Reverb"
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE)

target_sources(NimbusReverb
    PRIVATE
        Source/PluginEditor.cpp
        Source/PluginProcessor.cpp
        Source/ReverbEngine.cpp)

target_compile_definitions(NimbusReverb
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(NimbusReverb
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
