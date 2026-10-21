#include "PluginProcessor.h"

#include "Parameters.h"
#include "PluginEditor.h"

namespace
{
    constexpr double gainRampSeconds = 0.02;

    // Where file paths are kept in the saved state (DAW project or preset).
    const juce::Identifier ampModelPathKey { "ampModelPath" };
    const juce::Identifier cabIrPathKey { "cabIrPath" };

    juce::String describeModel (const AmpStage::ModelInfo& info)
    {
        // "48 kHz", "44.1 kHz"
        const auto kHz = info.modelSampleRate / 1000.0;
        const auto isWhole = std::abs (kHz - std::round (kHz)) < 0.05;
        auto detail = (isWhole ? juce::String (juce::roundToInt (kHz)) : juce::String (kHz, 1)) + " kHz model";
        if (info.resampling)
            detail << ", resampled";
        if (! info.hasLoudness)
            detail << ", no loudness data";
        return detail;
    }
}

ParagonX1AudioProcessor::ParagonX1AudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::mono(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "ParagonX1", createParameterLayout())
{
    inputGainDb = parameters.getRawParameterValue (ParamIDs::inputGain);
    outputGainDb = parameters.getRawParameterValue (ParamIDs::outputGain);
    ampOn = parameters.getRawParameterValue (ParamIDs::ampOn);
    ampNormalise = parameters.getRawParameterValue (ParamIDs::ampNormalise);
    cabOn = parameters.getRawParameterValue (ParamIDs::cabOn);

    // Housekeeping off the audio thread: free swapped-out models and keep the
    // host's latency figure up to date.
    startTimerHz (4);
}

ParagonX1AudioProcessor::~ParagonX1AudioProcessor()
{
    stopTimer();
    loader.removeAllJobs (true, 10000);
}

void ParagonX1AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    inputGain.reset (sampleRate, gainRampSeconds);
    outputGain.reset (sampleRate, gainRampSeconds);
    inputGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (inputGainDb->load()));
    outputGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (outputGainDb->load()));

    preparedBlockSize = juce::jmax (1, samplesPerBlock);
    amp.prepare (sampleRate, preparedBlockSize);
    cab.prepare (sampleRate, preparedBlockSize);
    compGate.prepare (sampleRate, preparedBlockSize);
    drive.prepare (sampleRate, preparedBlockSize);
    presence.prepare (sampleRate, preparedBlockSize);
    modulation.prepare (sampleRate, preparedBlockSize);
    delay.prepare (sampleRate, preparedBlockSize);
    reverb.prepare (sampleRate, preparedBlockSize);
    setLatencySamples (amp.getLatencySamples());
}

float ParagonX1AudioProcessor::param (const char* id) const
{
    // Looked up by ID each block. APVTS keeps these in a sorted map, so the
    // lookup is a handful of string compares and never allocates.
    return parameters.getRawParameterValue (id)->load();
}

void ParagonX1AudioProcessor::readHostTempo()
{
    if (auto* host = getPlayHead())
        if (const auto position = host->getPosition())
            if (const auto bpm = position->getBpm(); bpm.hasValue() && *bpm > 0.0)
                hostBpm = *bpm;
}

bool ParagonX1AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto isMonoOrStereo = [] (const juce::AudioChannelSet& set)
    {
        return set == juce::AudioChannelSet::mono() || set == juce::AudioChannelSet::stereo();
    };

    return isMonoOrStereo (layouts.getMainInputChannelSet())
        && isMonoOrStereo (layouts.getMainOutputChannelSet());
}

void ParagonX1AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (buffer.getNumChannels() == 0 || buffer.getNumSamples() == 0)
        return;

    readHostTempo();

    // Some hosts occasionally send more samples than they promised in
    // prepareToPlay. Every stage's scratch memory is sized for the promise,
    // so work through an oversized block in promised-size pieces.
    for (int start = 0; start < buffer.getNumSamples(); start += preparedBlockSize)
        processChunk (buffer, start, juce::jmin (preparedBlockSize, buffer.getNumSamples() - start));
}

void ParagonX1AudioProcessor::processChunk (juce::AudioBuffer<float>& buffer, int start, int numSamples)
{
    const auto numOutputChannels = juce::jmin (getTotalNumOutputChannels(), buffer.getNumChannels());

    // A guitar is a mono source, and NAM models are mono, so the chain up to
    // the cabinet runs on the first input channel. On a stereo track that is
    // the left channel, which is how most amp sims treat stereo input.
    auto* mono = buffer.getWritePointer (0, start);
    juce::dsp::AudioBlock<float> monoBlock (&mono, 1, (size_t) numSamples);

    inputGain.setTargetValue (juce::Decibels::decibelsToGain (inputGainDb->load()));
    inputGain.applyGain (mono, numSamples);
    inputMeter.pushSamples (mono, numSamples);

    compGate.process (monoBlock, { isOn (ParamIDs::compOn), param (ParamIDs::compThreshold), param (ParamIDs::compRatio),
                                   param (ParamIDs::compLevel), param (ParamIDs::gateThreshold) });
    drive.process (monoBlock, { isOn (ParamIDs::driveOn), param (ParamIDs::driveGain), param (ParamIDs::driveTone),
                                param (ParamIDs::driveLevel) });

    amp.process (mono, numSamples, ampOn->load() > 0.5f, ampNormalise->load() > 0.5f);
    cab.process (mono, numSamples, cabOn->load() > 0.5f);
    presence.process (monoBlock, param (ParamIDs::presence));

    // From here on, stereo: modulation, delay and reverb spread the sound
    // across both speakers.
    for (int channel = 1; channel < numOutputChannels; ++channel)
        buffer.copyFrom (channel, start, buffer, 0, start, numSamples);

    const auto stereoBlock = juce::dsp::AudioBlock<float> (buffer)
                                 .getSubBlock ((size_t) start, (size_t) numSamples)
                                 .getSubsetChannelBlock (0, (size_t) juce::jmin (numOutputChannels, effects::maxChannels));

    modulation.process (stereoBlock, { isOn (ParamIDs::modOn), (effects::Modulation::Type) (int) param (ParamIDs::modType),
                                       param (ParamIDs::modRate), param (ParamIDs::modDepth), param (ParamIDs::modMix) });
    delay.process (stereoBlock, { isOn (ParamIDs::delayOn), param (ParamIDs::delayTime), (int) param (ParamIDs::delaySync),
                                  param (ParamIDs::delayFeedback), param (ParamIDs::delayTone), param (ParamIDs::delayMix),
                                  isOn (ParamIDs::delayPingPong) },
                   hostBpm.load());
    reverb.process (stereoBlock, { isOn (ParamIDs::reverbOn), param (ParamIDs::reverbSize), param (ParamIDs::reverbTone),
                                   param (ParamIDs::reverbMix) });

    outputGain.setTargetValue (juce::Decibels::decibelsToGain (outputGainDb->load()));
    for (int i = 0; i < numSamples; ++i)
    {
        const auto gain = outputGain.getNextValue();
        for (size_t channel = 0; channel < stereoBlock.getNumChannels(); ++channel)
            stereoBlock.getChannelPointer (channel)[i] *= gain;
    }

    for (size_t channel = 0; channel < stereoBlock.getNumChannels(); ++channel)
        outputMeter.pushSamples (stereoBlock.getChannelPointer (channel), numSamples);
}

//==============================================================================
void ParagonX1AudioProcessor::setStatus (SlotStatus& slot, SlotStatus newStatus)
{
    {
        const juce::ScopedLock lock (statusLock);
        slot = std::move (newStatus);
    }
    statusBroadcaster.sendChangeMessage();
}

SlotStatus ParagonX1AudioProcessor::getAmpStatus() const
{
    const juce::ScopedLock lock (statusLock);
    return ampStatus;
}

SlotStatus ParagonX1AudioProcessor::getCabStatus() const
{
    const juce::ScopedLock lock (statusLock);
    return cabStatus;
}

void ParagonX1AudioProcessor::loadAmpModel (const juce::File& file)
{
    auto loading = getAmpStatus();
    loading.loading = true;
    setStatus (ampStatus, loading);

    loader.addJob ([this, file]
    {
        auto status = getAmpStatus();
        status.loading = false;

        if (! file.existsAsFile())
        {
            // Usually a project opened on a computer without this model.
            // Keep the path so saving the project doesn't lose it.
            status.file = file;
            status.detail = {};
            status.error = "Can't find " + file.getFullPathName();
            setStatus (ampStatus, status);
            return;
        }

        try
        {
            // Parsing and preparing happen here, off the audio thread.
            const auto info = amp.stage (std::make_unique<AmpModel> (file));
            status.file = file;
            status.detail = describeModel (info);
            status.error = {};
        }
        catch (const std::exception& e)
        {
            // Keep the previous model playing; just report the problem.
            status.error = file.getFileName() + ": " + e.what();
        }

        setStatus (ampStatus, status);
    });
}

void ParagonX1AudioProcessor::clearAmpModel()
{
    loader.addJob ([this]
    {
        amp.stage (nullptr);
        setStatus (ampStatus, {});
    });
}

void ParagonX1AudioProcessor::loadCabinetIR (const juce::File& file)
{
    auto loading = getCabStatus();
    loading.loading = true;
    setStatus (cabStatus, loading);

    loader.addJob ([this, file]
    {
        auto status = getCabStatus();
        status.loading = false;

        if (! file.existsAsFile())
        {
            status.file = file;   // keep the path, as for the amp
            status.detail = {};
            status.error = "Can't find " + file.getFullPathName();
            setStatus (cabStatus, status);
            return;
        }

        if (const auto result = cab.load (file); result.error.isNotEmpty())
        {
            // Keep the previous IR; just report the problem.
            status.error = file.getFileName() + ": " + result.error;
        }
        else
        {
            status.file = file;
            status.error = {};
            status.detail = juce::String (juce::roundToInt (result.lengthSeconds * 1000.0)) + " ms IR";
        }

        setStatus (cabStatus, status);
    });
}

void ParagonX1AudioProcessor::clearCabinetIR()
{
    loader.addJob ([this]
    {
        cab.clear();
        setStatus (cabStatus, {});
    });
}

bool ParagonX1AudioProcessor::waitForLoads (int timeoutMs)
{
    const auto deadline = juce::Time::getMillisecondCounter() + (juce::uint32) timeoutMs;
    while (loader.getNumJobs() > 0)
    {
        if (juce::Time::getMillisecondCounter() > deadline)
            return false;
        juce::Thread::sleep (5);
    }
    return true;
}

void ParagonX1AudioProcessor::timerCallback()
{
    amp.collectGarbage();

    if (const auto latency = amp.getLatencySamples(); latency != getLatencySamples())
        setLatencySamples (latency);
}

//==============================================================================
juce::AudioProcessorEditor* ParagonX1AudioProcessor::createEditor()
{
    return new ParagonX1AudioProcessorEditor (*this);
}

void ParagonX1AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    state.setProperty (ampModelPathKey, getAmpStatus().file.getFullPathName(), nullptr);
    state.setProperty (cabIrPathKey, getCabStatus().file.getFullPathName(), nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void ParagonX1AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (parameters.state.getType()))
        return;

    const auto state = juce::ValueTree::fromXml (*xml);
    parameters.replaceState (state);

    const auto restore = [] (const juce::var& path, auto load, auto clear)
    {
        if (const auto pathString = path.toString(); pathString.isNotEmpty())
            load (juce::File (pathString));
        else
            clear();
    };

    restore (state[ampModelPathKey], [this] (auto f) { loadAmpModel (f); }, [this] { clearAmpModel(); });
    restore (state[cabIrPathKey], [this] (auto f) { loadCabinetIR (f); }, [this] { clearCabinetIR(); });
}

// JUCE calls this to create new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ParagonX1AudioProcessor();
}
