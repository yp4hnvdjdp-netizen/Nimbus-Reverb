#pragma once

#include "PluginProcessor.h"

#include <functional>

class NimbusLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    NimbusLookAndFeel();

    void drawRotarySlider (juce::Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider&) override;

    void drawButtonBackground (juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

private:
    juce::Colour accentColour;
    juce::Colour surfaceColour;
};

class KnobControl final : public juce::Component
{
public:
    using Formatter = std::function<juce::String (double)>;

    KnobControl (juce::AudioProcessorValueTreeState& state,
                 const juce::String& parameterID,
                 const juce::String& labelText,
                 Formatter textFormatter,
                 juce::LookAndFeel* lookAndFeel);

    void resized() override;

private:
    juce::Label label;
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

class NimbusReverbAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit NimbusReverbAudioProcessorEditor (NimbusReverbAudioProcessor&);
    ~NimbusReverbAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    NimbusReverbAudioProcessor& processorRef;
    NimbusLookAndFeel lookAndFeel;
    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::TextButton freezeButton { "Freeze" };
    std::unique_ptr<KnobControl> mixControl;
    std::unique_ptr<KnobControl> sizeControl;
    std::unique_ptr<KnobControl> preDelayControl;
    std::unique_ptr<KnobControl> dampingControl;
    std::unique_ptr<KnobControl> widthControl;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> freezeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NimbusReverbAudioProcessorEditor)
};
