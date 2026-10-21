#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "LevelMeterSource.h"
#include "dsp/AmpStage.h"
#include "dsp/CabStage.h"

// What the editor shows for the amp or cabinet slot.
struct SlotStatus
{
    juce::File file;        // the file in use (or that a saved project asked for)
    juce::String detail;    // e.g. "48 kHz, resampled" or the IR length
    juce::String error;     // why the last load failed; empty if it didn't
    bool loading = false;
};

class ParagonX1AudioProcessor final : public juce::AudioProcessor,
                                      private juce::Timer
{
public:
    ParagonX1AudioProcessor();
    ~ParagonX1AudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // Presets live in the rig browser (Phase 4), so the host's program list is a single entry.
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Loading happens on a background thread; these return immediately.
    // Listen to statusBroadcaster to hear when a load finishes.
    void loadAmpModel (const juce::File& file);
    void clearAmpModel();
    void loadCabinetIR (const juce::File& file);
    void clearCabinetIR();

    SlotStatus getAmpStatus() const;
    SlotStatus getCabStatus() const;
    juce::ChangeBroadcaster statusBroadcaster;

    // Blocks until queued loads finish. For tests.
    bool waitForLoads (int timeoutMs);

    juce::AudioProcessorValueTreeState& getParameters() noexcept { return parameters; }
    LevelMeterSource& getInputMeter() noexcept { return inputMeter; }
    LevelMeterSource& getOutputMeter() noexcept { return outputMeter; }

private:
    void timerCallback() override;
    void setStatus (SlotStatus& slot, SlotStatus newStatus);

    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* inputGainDb = nullptr;
    std::atomic<float>* outputGainDb = nullptr;
    std::atomic<float>* ampOn = nullptr;
    std::atomic<float>* ampNormalise = nullptr;
    std::atomic<float>* cabOn = nullptr;

    // Smoothing stops zipper noise when a gain knob moves.
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> inputGain, outputGain;

    AmpStage amp;
    CabStage cab;

    juce::CriticalSection statusLock;
    SlotStatus ampStatus, cabStatus;

    LevelMeterSource inputMeter, outputMeter;

    // Declared last so it is destroyed first: its jobs use the members above.
    juce::ThreadPool loader { juce::ThreadPoolOptions{}.withThreadName ("Paragon X1 loader").withNumberOfThreads (1) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParagonX1AudioProcessor)
};
