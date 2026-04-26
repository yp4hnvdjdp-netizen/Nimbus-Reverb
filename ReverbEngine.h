#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace nimbus
{
struct ReverbSettings
{
    float mix = 0.28f;
    float size = 0.62f;
    float preDelayMs = 18.0f;
    float damping = 0.44f;
    float width = 1.0f;
    bool freeze = false;
};

class ReverbEngine final
{
public:
    void prepare (double sampleRate, int samplesPerBlockExpected, int numChannels);
    void reset();

    void process (juce::AudioBuffer<float>& buffer, const ReverbSettings& settings);

    [[nodiscard]] double getTailLengthSeconds() const noexcept { return 12.0; }

private:
    static float mapSizeToRoomSize (float size) noexcept;

    void processChunk (juce::AudioBuffer<float>& buffer,
                       int startSample,
                       int numSamples,
                       int numChannels);

    double sampleRate = 44100.0;
    int maxChunkSize = 0;
    int numPreparedChannels = 2;
    int maxPreDelaySamples = 1;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> preDelay;
    juce::Reverb reverb;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoother;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delaySmoother;
    juce::AudioBuffer<float> wetBuffer;
};
} // namespace nimbus
