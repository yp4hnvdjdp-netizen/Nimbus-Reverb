#include "PluginProcessor.h"
#include "PluginEditor.h"

NimbusReverbAudioProcessor::NimbusReverbAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", nimbus::parameters::createLayout())
{
}

NimbusReverbAudioProcessor::~NimbusReverbAudioProcessor() = default;

const juce::String NimbusReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NimbusReverbAudioProcessor::acceptsMidi() const
{
    return false;
}

bool NimbusReverbAudioProcessor::producesMidi() const
{
    return false;
}

bool NimbusReverbAudioProcessor::isMidiEffect() const
{
    return false;
}

double NimbusReverbAudioProcessor::getTailLengthSeconds() const
{
    return engine.getTailLengthSeconds();
}

int NimbusReverbAudioProcessor::getNumPrograms()
{
    return 1;
}

int NimbusReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NimbusReverbAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String NimbusReverbAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void NimbusReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void NimbusReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock, getTotalNumInputChannels());
}

void NimbusReverbAudioProcessor::releaseResources()
{
    engine.reset();
}

bool NimbusReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

nimbus::ReverbSettings NimbusReverbAudioProcessor::snapshotParameters() const
{
    auto* mix = apvts.getRawParameterValue (nimbus::parameters::mixId);
    auto* size = apvts.getRawParameterValue (nimbus::parameters::sizeId);
    auto* preDelay = apvts.getRawParameterValue (nimbus::parameters::preDelayId);
    auto* damping = apvts.getRawParameterValue (nimbus::parameters::dampingId);
    auto* width = apvts.getRawParameterValue (nimbus::parameters::widthId);
    auto* freeze = apvts.getRawParameterValue (nimbus::parameters::freezeId);

    nimbus::ReverbSettings settings;
    settings.mix = mix != nullptr ? mix->load() : settings.mix;
    settings.size = size != nullptr ? size->load() : settings.size;
    settings.preDelayMs = preDelay != nullptr ? preDelay->load() : settings.preDelayMs;
    settings.damping = damping != nullptr ? damping->load() : settings.damping;
    settings.width = width != nullptr ? width->load() : settings.width;
    settings.freeze = freeze != nullptr ? freeze->load() >= 0.5f : settings.freeze;
    return settings;
}

void NimbusReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear (channel, 0, buffer.getNumSamples());

    engine.process (buffer, snapshotParameters());
}

juce::AudioProcessorEditor* NimbusReverbAudioProcessor::createEditor()
{
    return new NimbusReverbAudioProcessorEditor (*this);
}

void NimbusReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void NimbusReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NimbusReverbAudioProcessor();
}
