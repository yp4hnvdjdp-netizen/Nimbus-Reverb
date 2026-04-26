#include "PluginEditor.h"

namespace
{
juce::Colour panelBase()         { return juce::Colour (0xff0b1017); }
juce::Colour panelTop()          { return juce::Colour (0xff111a28); }
juce::Colour accent()            { return juce::Colour (0xff6fd3ff); }
juce::Colour accentDimmed()      { return juce::Colour (0xff29506f); }
juce::Colour textPrimary()       { return juce::Colour (0xfff6fbff); }
juce::Colour textSecondary()     { return juce::Colour (0xffa8b6c8); }
}

NimbusLookAndFeel::NimbusLookAndFeel()
{
    accentColour = accent();
    surfaceColour = panelTop();
}

void NimbusLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                          int x,
                                          int y,
                                          int width,
                                          int height,
                                          float sliderPos,
                                          float rotaryStartAngle,
                                          float rotaryEndAngle,
                                          juce::Slider&)
{
    auto bounds = juce::Rectangle<float> (x, y, width, height).reduced (4.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto arcRadius = radius * 0.72f;

    g.setColour (juce::Colours::black.withAlpha (0.22f));
    g.fillEllipse (bounds.translated (0.0f, 4.0f));

    g.setColour (surfaceColour.withAlpha (0.95f));
    g.fillEllipse (bounds);

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.strokePath (track, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, angle, true);
    g.setColour (accentColour);
    g.strokePath (valueArc, juce::PathStrokeType (4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour (juce::Colours::white.withAlpha (0.11f));
    g.fillEllipse (centre.x - radius * 0.30f, centre.y - radius * 0.30f, radius * 0.60f, radius * 0.60f);

    const auto thumbAngle = angle - juce::MathConstants<float>::halfPi;
    const auto thumbLength = radius * 0.58f;
    const auto thumbWidth = radius * 0.13f;
    const auto x1 = centre.x + std::cos (thumbAngle) * thumbLength;
    const auto y1 = centre.y + std::sin (thumbAngle) * thumbLength;
    const auto x2 = centre.x + std::cos (thumbAngle) * (thumbLength - thumbWidth);
    const auto y2 = centre.y + std::sin (thumbAngle) * (thumbLength - thumbWidth);

    g.setColour (juce::Colours::white);
    g.drawLine ({ x2, y2, x1, y1 }, thumbWidth);
}

void NimbusLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                              juce::Button& button,
                                              const juce::Colour&,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    const auto radius = 14.0f;
    const auto active = button.getToggleState();
    const auto base = active ? accentColour : surfaceColour.brighter (0.04f);
    const auto overlay = shouldDrawButtonAsDown ? juce::Colours::white.withAlpha (0.12f)
                                                : shouldDrawButtonAsHighlighted ? juce::Colours::white.withAlpha (0.05f)
                                                                                : juce::Colours::transparentWhite;

    g.setColour (juce::Colours::black.withAlpha (0.22f));
    g.fillRoundedRectangle (bounds.translated (0.0f, 4.0f), radius);

    g.setColour (base);
    g.fillRoundedRectangle (bounds, radius);

    g.setColour (overlay);
    g.fillRoundedRectangle (bounds, radius);

    g.setColour (juce::Colours::white.withAlpha (active ? 0.28f : 0.12f));
    g.drawRoundedRectangle (bounds, radius, 1.0f);
}

KnobControl::KnobControl (juce::AudioProcessorValueTreeState& state,
                          const juce::String& parameterID,
                          const juce::String& labelText,
                          Formatter textFormatter,
                          juce::LookAndFeel* lookAndFeel)
{
    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, textPrimary());
    label.setFont (juce::Font (juce::FontOptions (15.0f)));
    addAndMakeVisible (label);

    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 20);
    slider.setLookAndFeel (lookAndFeel);
    slider.textFromValueFunction = [formatter = std::move (textFormatter)] (double value)
    {
        return formatter (value);
    };
    slider.setColour (juce::Slider::textBoxTextColourId, textPrimary());
    slider.setColour (juce::Slider::textBoxBackgroundColourId, panelBase().withAlpha (0.88f));
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::rotarySliderFillColourId, accent());
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, accentDimmed());
    addAndMakeVisible (slider);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, parameterID, slider);
}

void KnobControl::resized()
{
    auto area = getLocalBounds();
    label.setBounds (area.removeFromTop (20));
    area.removeFromTop (4);
    slider.setBounds (area);
}

NimbusReverbAudioProcessorEditor::NimbusReverbAudioProcessorEditor (NimbusReverbAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&lookAndFeel);
    setOpaque (true);
    setSize (820, 500);

    titleLabel.setText ("Nimbus Reverb", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, textPrimary());
    titleLabel.setFont (juce::Font (juce::FontOptions (30.0f, juce::Font::bold)));
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("Stereo bloom with pre-delay, damping, width, and freeze mode.", juce::dontSendNotification);
    subtitleLabel.setJustificationType (juce::Justification::centred);
    subtitleLabel.setColour (juce::Label::textColourId, textSecondary());
    subtitleLabel.setFont (juce::Font (juce::FontOptions (14.0f)));
    addAndMakeVisible (subtitleLabel);

    auto& state = processorRef.apvts;

    mixControl = std::make_unique<KnobControl> (state,
                                                nimbus::parameters::mixId,
                                                "Mix",
                                                [] (double value) { return juce::String (juce::roundToInt (value * 100.0)) + "%"; },
                                                &lookAndFeel);

    sizeControl = std::make_unique<KnobControl> (state,
                                                 nimbus::parameters::sizeId,
                                                 "Size",
                                                 [] (double value) { return juce::String (juce::roundToInt (value * 100.0)) + "%"; },
                                                 &lookAndFeel);

    preDelayControl = std::make_unique<KnobControl> (state,
                                                     nimbus::parameters::preDelayId,
                                                     "Pre-Delay",
                                                     [] (double value) { return juce::String (value, 1) + " ms"; },
                                                     &lookAndFeel);

    dampingControl = std::make_unique<KnobControl> (state,
                                                    nimbus::parameters::dampingId,
                                                    "Damping",
                                                    [] (double value) { return juce::String (juce::roundToInt (value * 100.0)) + "%"; },
                                                    &lookAndFeel);

    widthControl = std::make_unique<KnobControl> (state,
                                                  nimbus::parameters::widthId,
                                                  "Width",
                                                  [] (double value) { return juce::String (juce::roundToInt (value * 100.0)) + "%"; },
                                                  &lookAndFeel);

    addAndMakeVisible (*mixControl);
    addAndMakeVisible (*sizeControl);
    addAndMakeVisible (*preDelayControl);
    addAndMakeVisible (*dampingControl);
    addAndMakeVisible (*widthControl);

    freezeButton.setClickingTogglesState (true);
    freezeButton.setColour (juce::TextButton::buttonColourId, panelTop());
    freezeButton.setColour (juce::TextButton::buttonOnColourId, accent());
    freezeButton.setColour (juce::TextButton::textColourOnId, textPrimary());
    freezeButton.setColour (juce::TextButton::textColourOffId, textPrimary());
    freezeButton.setLookAndFeel (&lookAndFeel);
    addAndMakeVisible (freezeButton);

    freezeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state,
                                                                                              nimbus::parameters::freezeId,
                                                                                              freezeButton);
}

NimbusReverbAudioProcessorEditor::~NimbusReverbAudioProcessorEditor()
{
    freezeButton.setLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

void NimbusReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient gradient (panelTop(), bounds.getCentreX(), bounds.getY(), panelBase(), bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill (gradient);
    g.fillAll();

    g.setColour (accent().withAlpha (0.08f));
    g.fillEllipse (bounds.getWidth() * 0.08f, bounds.getHeight() * 0.00f, bounds.getWidth() * 0.42f, bounds.getHeight() * 0.35f);
    g.fillEllipse (bounds.getWidth() * 0.56f, bounds.getHeight() * 0.56f, bounds.getWidth() * 0.36f, bounds.getHeight() * 0.28f);

    auto panel = getLocalBounds().reduced (18).toFloat();
    g.setColour (juce::Colours::white.withAlpha (0.04f));
    g.fillRoundedRectangle (panel, 22.0f);
    g.setColour (juce::Colours::white.withAlpha (0.09f));
    g.drawRoundedRectangle (panel, 22.0f, 1.0f);
}

void NimbusReverbAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (34);

    auto header = area.removeFromTop (82);
    titleLabel.setBounds (header.removeFromTop (34));
    header.removeFromTop (4);
    subtitleLabel.setBounds (header.removeFromTop (18));

    area.removeFromTop (10);

    juce::Grid grid;
    grid.rowGap = juce::Grid::Px (18.0f);
    grid.columnGap = juce::Grid::Px (18.0f);
    grid.templateColumns = { juce::Grid::Fr (1), juce::Grid::Fr (1), juce::Grid::Fr (1) };
    grid.templateRows = { juce::Grid::Fr (1), juce::Grid::Fr (1) };

    grid.items = { juce::GridItem (*mixControl),
                   juce::GridItem (*sizeControl),
                   juce::GridItem (*preDelayControl),
                   juce::GridItem (*dampingControl),
                   juce::GridItem (*widthControl),
                   juce::GridItem (freezeButton) };

    grid.performLayout (area);
}
