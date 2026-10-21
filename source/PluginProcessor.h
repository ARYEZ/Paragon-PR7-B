#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "LevelMeterSource.h"
#include "dsp/AmpStage.h"
#include "dsp/CabStage.h"
#include "dsp/effects/CompGate.h"
#include "dsp/effects/Delay.h"
#include "dsp/effects/Drive.h"
#include "dsp/effects/Modulation.h"
#include "dsp/effects/Presence.h"
#include "dsp/effects/Reverb.h"
#include "rigs/RigLibrary.h"

// What the editor shows for the amp or cabinet slot.
struct SlotStatus
{
    juce::File file;        // the file in use (or that a saved project asked for)
    juce::String detail;    // e.g. "48 kHz, resampled" or the IR length
    juce::String error;     // why the last load failed; empty if it didn't
    bool loading = false;
};

// Which rig is loaded, for the header display and the Save button.
struct CurrentRig
{
    juce::String name { "Untitled" };
    juce::File file;              // set for the user's own rigs
    juce::String factoryName;     // set for factory rigs
    bool isUserRig() const { return file != juce::File(); }
};

class ParagonX1AudioProcessor final : public juce::AudioProcessor,
                                      private juce::Timer,
                                      private juce::AudioProcessorValueTreeState::Listener
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

    //==========================================================================
    // Rigs (message thread only)
    RigLibrary& getRigLibrary() noexcept { return *rigLibrary; }

    // Points the library at another folder. For tests.
    void useRigFolder (const juce::File& folder);

    // The current sound as a rig (not saved anywhere yet).
    Rig captureRig() const;

    void loadRig (const RigLibrary::Entry& entry);

    // Saves over the current rig if it's one of the user's own and returns
    // true; returns false if it isn't (a factory rig or never saved), in
    // which case the caller should ask for a name and use saveCurrentRigAs.
    bool saveCurrentRig();
    bool saveCurrentRigAs (const juce::String& name);

    // Loads the next (+1) or previous (-1) rig in the library.
    void stepRig (int direction);

    // Rename, re-file or re-tag a user rig from the browser.
    bool updateRigDetails (const RigLibrary::Entry& entry, const Rig& newDetails);
    bool deleteRig (const RigLibrary::Entry& entry);

    CurrentRig getCurrentRig() const { return currentRig; }
    const RigLibrary::Entry* findCurrentRigEntry() const;

    // True once anything in the rig has changed since it was loaded or saved.
    bool isRigModified() const noexcept { return rigModified.load(); }

    // Tells listeners (the editor) that the rig list or current rig changed.
    juce::ChangeBroadcaster rigBroadcaster;

    //==========================================================================
    // The DAW's tempo, as last seen by the audio thread (120 if it hasn't said).
    double getHostBpm() const noexcept { return hostBpm.load(); }

    juce::AudioProcessorValueTreeState& getParameters() noexcept { return parameters; }
    LevelMeterSource& getInputMeter() noexcept { return inputMeter; }
    LevelMeterSource& getOutputMeter() noexcept { return outputMeter; }

private:
    void processChunk (juce::AudioBuffer<float>& buffer, int start, int numSamples);
    void readHostTempo();
    float param (const char* id) const;
    bool isOn (const char* id) const { return param (id) > 0.5f; }

    void timerCallback() override;
    void setStatus (SlotStatus& slot, SlotStatus newStatus);
    void parameterChanged (const juce::String& parameterId, float newValue) override;
    void markModified() noexcept;
    void applyRig (const Rig& rig);

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

    // Signal chain, in order: compGate -> drive -> amp -> cab -> presence
    // -> modulation -> delay -> reverb. Up to the cab it's mono; from
    // modulation on it's stereo when the track is.
    effects::CompGate compGate;
    effects::Drive drive;
    effects::Presence presence;
    effects::Modulation modulation;
    effects::Delay delay;
    effects::Reverb reverb;

    std::unique_ptr<RigLibrary> rigLibrary;
    CurrentRig currentRig;
    // Atomic because the DAW's automation changes parameters on the audio thread.
    std::atomic<bool> rigModified { false }, applyingRig { false };

    int preparedBlockSize = 512;
    std::atomic<double> hostBpm { 120.0 };

    juce::CriticalSection statusLock;
    SlotStatus ampStatus, cabStatus;

    LevelMeterSource inputMeter, outputMeter;

    // Declared last so it is destroyed first: its jobs use the members above.
    juce::ThreadPool loader { juce::ThreadPoolOptions{}.withThreadName ("Paragon X1 loader").withNumberOfThreads (1) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParagonX1AudioProcessor)
};
