#include "ReverbEngine.h"

#include <cmath>

namespace nimbus
{
static float clamp01 (float value) noexcept
{
    return juce::jlimit (0.0f, 1.0f, value);
}

float ReverbEngine::mapSizeToRoomSize (float size) noexcept
{
    const auto curved = std::pow (clamp01 (size), 1.35f);
    return juce::jmap (curved, 0.0f, 1.0f, 0.08f, 0.98f);
}

void ReverbEngine::prepare (double newSampleRate, int samplesPerBlockExpected, int numChannels)
{
    sampleRate = newSampleRate;
    numPreparedChannels = juce::jlimit (1, 2, numChannels);
    maxChunkSize = juce::jmax (1, samplesPerBlockExpected);
    maxPreDelaySamples = juce::jmax (1, (int) std::ceil (sampleRate * 0.18));

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) maxChunkSize;
    spec.numChannels = (juce::uint32) numPreparedChannels;

    preDelay.prepare (spec);
    preDelay.setMaximumDelayInSamples (maxPreDelaySamples);

    reverb.setSampleRate (sampleRate);
    reverb.reset();

    mixSmoother.reset (sampleRate, 0.02);
    delaySmoother.reset (sampleRate, 0.025);
    mixSmoother.setCurrentAndTargetValue (0.28f);
    delaySmoother.setCurrentAndTargetValue ((float) (18.0 * sampleRate / 1000.0));

    wetBuffer.setSize (numPreparedChannels, juce::jmax (maxChunkSize, 2048), false, false, true);
    wetBuffer.clear();
}

void ReverbEngine::reset()
{
    preDelay.reset();
    reverb.reset();
    mixSmoother.reset (sampleRate, 0.02);
    delaySmoother.reset (sampleRate, 0.025);
}

void ReverbEngine::process (juce::AudioBuffer<float>& buffer, const ReverbSettings& settings)
{
    const auto numChannels = juce::jmin (buffer.getNumChannels(), numPreparedChannels);
    const auto totalSamples = buffer.getNumSamples();

    if (numChannels <= 0 || totalSamples <= 0)
        return;

    juce::Reverb::Parameters params;
    params.roomSize = mapSizeToRoomSize (settings.size);
    params.damping = clamp01 (settings.damping);
    params.wetLevel = 1.0f;
    params.dryLevel = 0.0f;
    params.width = clamp01 (settings.width);
    params.freezeMode = settings.freeze ? 1.0f : 0.0f;
    reverb.setParameters (params);

    mixSmoother.setTargetValue (clamp01 (settings.mix));
    delaySmoother.setTargetValue ((float) (juce::jlimit (0.0, (double) maxPreDelaySamples, settings.preDelayMs * sampleRate / 1000.0)));

    int remaining = totalSamples;
    int startSample = 0;

    while (remaining > 0)
    {
        const auto chunk = juce::jmin (wetBuffer.getNumSamples(), remaining);
        processChunk (buffer, startSample, chunk, numChannels);
        startSample += chunk;
        remaining -= chunk;
    }
}

void ReverbEngine::processChunk (juce::AudioBuffer<float>& buffer,
                                 int startSample,
                                 int numSamples,
                                 int numChannels)
{
    jassert (numSamples <= wetBuffer.getNumSamples());
    jassert (numChannels <= wetBuffer.getNumChannels());

    auto* wetLeft = wetBuffer.getWritePointer (0);
    auto* wetRight = numChannels > 1 ? wetBuffer.getWritePointer (1) : nullptr;
    auto* outLeft = buffer.getWritePointer (0) + startSample;
    auto* outRight = numChannels > 1 ? buffer.getWritePointer (1) + startSample : nullptr;
    const auto* inLeft = buffer.getReadPointer (0) + startSample;
    const auto* inRight = numChannels > 1 ? buffer.getReadPointer (1) + startSample : nullptr;

    wetBuffer.clear (0, 0, numSamples);
    if (numChannels > 1)
        wetBuffer.clear (1, 0, numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        const auto currentDelay = juce::jlimit (0.0f, (float) maxPreDelaySamples, delaySmoother.getNextValue());
        preDelay.setDelay (currentDelay);

        preDelay.pushSample (0, inLeft[i]);
        wetLeft[i] = preDelay.popSample (0);

        if (numChannels > 1)
        {
            preDelay.pushSample (1, inRight[i]);
            wetRight[i] = preDelay.popSample (1);
        }
    }

    if (numChannels > 1)
        reverb.processStereo (wetLeft, wetRight, numSamples);
    else
        reverb.processMono (wetLeft, numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        const auto mix = mixSmoother.getNextValue();
        const auto dryGain = 1.0f - mix;
        const auto wetGain = mix;

        outLeft[i] = inLeft[i] * dryGain + wetLeft[i] * wetGain;

        if (numChannels > 1)
            outRight[i] = inRight[i] * dryGain + wetRight[i] * wetGain;
    }
}
} // namespace nimbus
