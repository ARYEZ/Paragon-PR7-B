#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "PluginProcessor.h"
#include "ui/CurrentRigPanel.h"
#include "ui/ParagonLookAndFeel.h"
#include "ui/EffectPanel.h"
#include "ui/Footswitch.h"
#include "ui/LedDisplay.h"
#include "ui/LevelMeter.h"
#include "ui/ModelSlot.h"
#include "ui/PedalBody.h"
#include "ui/RigBrowser.h"
#include "ui/RigSlot.h"
#include "ui/SignalChain.h"

// A side strip from the mockup: a title, a gain knob with its value
// underneath, and a level meter.
class GainColumn final : public juce::Component
{
public:
    GainColumn (const juce::String& title, juce::AudioProcessorValueTreeState& parameters,
                const juce::String& parameterId, LevelMeterSource& meterSource);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Label titleLabel;
    juce::Slider knob { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
    LevelMeter meter;
};

// Everything in the plugin window, laid out on a fixed 1600 x 900 canvas
// with the mockup's proportions. The editor scales this whole view to the
// window size, so the layout is the same at every size and stays sharp
// (everything is drawn as vectors).
//
//   header:  logo | PROCESSOR / PRESETS | < rig > | tempo | SAVE
//   middle:  INPUT strip | the pedal unit | OUTPUT strip
//   dock:    the selected block's knobs | CURRENT RIG
//   bottom:  SIGNAL CHAIN
class MainView final : public juce::Component,
                       private juce::ChangeListener,
                       private juce::Timer
{
public:
    explicit MainView (ParagonX1AudioProcessor&);
    ~MainView() override;

    static constexpr int width = 1600;
    static constexpr int height = 900;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // The blocks in signal-chain order; footswitches and the chain strip
    // select these to show their knobs.
    enum Block { compBlock, driveBlock, ampBlock, cabBlock, modBlock, delayBlock, reverbBlock, numBlocks };

    void changeListenerCallback (juce::ChangeBroadcaster*) override;
    void timerCallback() override;

    void selectBlock (int block);
    void showPage (bool presets);
    void refreshRig();
    void saveRig();
    juce::String valueText (const char* parameterId) const;

    ParagonX1AudioProcessor& audioProcessor;

    // Header
    juce::TextButton processorTab { "PROCESSOR" }, presetsTab { "PRESETS" };
    juce::TextButton previousRig { "<" }, nextRig { ">" }, rigNameButton, saveButton { "SAVE" };
    juce::Label tempoLabel;
    juce::Rectangle<int> headerArea, dockArea;

    GainColumn inputColumn, outputColumn;

    // The pedal unit and what sits on it
    PedalBody pedal;
    LedDisplay display;
    juce::ArrowButton rigUp { "Previous rig", 0.75f, ParagonColours::ink }, rigDown { "Next rig", 0.25f, ParagonColours::ink };
    juce::TextButton pedalSave { "SAVE" }, pedalSaveAs { "SAVE AS..." }, bypassButton;
    juce::AudioProcessorValueTreeState::ButtonAttachment bypassAttachment;
    juce::Slider rigLevelKnob { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox },
                 presenceKnob { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox };
    juce::AudioProcessorValueTreeState::SliderAttachment rigLevelAttachment, presenceAttachment;
    juce::Label bypassLabel { {}, "BYPASS" }, rigLevelLabel { {}, "OUTPUT LEVEL" }, presenceLabel { {}, "PRESENCE" };
    ModelSlot ampSlot, cabSlot;
    RigSlot rigSlot;
    juce::ToggleButton normaliseButton { "NORMALISE" };
    juce::AudioProcessorValueTreeState::ButtonAttachment normaliseAttachment;
    std::array<std::unique_ptr<Footswitch>, 4> footswitches;

    // Dock and signal chain
    std::array<std::unique_ptr<EffectPanel>, numBlocks> panels;
    CurrentRigPanel currentRigPanel;
    SignalChain signalChain;
    int selectedBlock = compBlock;

    // PRESETS page
    RigBrowser rigBrowser;
    std::vector<juce::Component*> processorPage;
    bool showingPresets = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainView)
};
