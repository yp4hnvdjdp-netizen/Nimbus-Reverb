#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace nimbus::parameters
{
inline constexpr auto mixId = "mix";
inline constexpr auto sizeId = "size";
inline constexpr auto preDelayId = "predelay";
inline constexpr auto dampingId = "damping";
inline constexpr auto widthId = "width";
inline constexpr auto freezeId = "freeze";

inline juce::String percentToString (float value, int)
{
    return juce::String (juce::roundToInt (value * 100.0f)) + "%";
}

inline float percentFromString (const juce::String& text)
{
    return text.retainCharacters ("0123456789.-").getFloatValue() / 100.0f;
}

inline juce::String msToString (float value, int)
{
    return juce::String (value, 1) + " ms";
}

inline float msFromString (const juce::String& text)
{
    return text.retainCharacters ("0123456789.-").getFloatValue();
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using Parameter = juce::AudioProcessorValueTreeState::Parameter;
    using Attributes = juce::AudioProcessorValueTreeStateParameterAttributes;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<Parameter> (juce::ParameterID { mixId, 1 },
                                             "Mix",
                                             juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f },
                                             0.28f,
                                             Attributes().withLabel ("%")
                                                         .withStringFromValueFunction (percentToString)
                                                         .withValueFromStringFunction (percentFromString)));

    layout.add (std::make_unique<Parameter> (juce::ParameterID { sizeId, 1 },
                                             "Size",
                                             juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f },
                                             0.62f,
                                             Attributes().withLabel ("%")
                                                         .withStringFromValueFunction (percentToString)
                                                         .withValueFromStringFunction (percentFromString)));

    layout.add (std::make_unique<Parameter> (juce::ParameterID { preDelayId, 1 },
                                             "Pre-Delay",
                                             juce::NormalisableRange<float> { 0.0f, 180.0f, 0.01f },
                                             18.0f,
                                             Attributes().withLabel ("ms")
                                                         .withStringFromValueFunction (msToString)
                                                         .withValueFromStringFunction (msFromString)));

    layout.add (std::make_unique<Parameter> (juce::ParameterID { dampingId, 1 },
                                             "Damping",
                                             juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f },
                                             0.44f,
                                             Attributes().withLabel ("%")
                                                         .withStringFromValueFunction (percentToString)
                                                         .withValueFromStringFunction (percentFromString)));

    layout.add (std::make_unique<Parameter> (juce::ParameterID { widthId, 1 },
                                             "Width",
                                             juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f },
                                             1.0f,
                                             Attributes().withLabel ("%")
                                                         .withStringFromValueFunction (percentToString)
                                                         .withValueFromStringFunction (percentFromString)));

    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { freezeId, 1 },
                                                            "Freeze",
                                                            false));

    return layout;
}
} // namespace nimbus::parameters
