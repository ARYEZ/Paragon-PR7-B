#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "PluginProcessor.h"
#include "ui/EffectPanel.h"
#include "ui/Footswitch.h"
#include "ui/LevelMeter.h"
#include "ui/ModelSlot.h"
#include "ui/ParagonLookAndFeel.h"

// One side column of the mockup: a title, a gain knob with its value
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

class ParagonX1AudioProcessorEditor final : public juce::AudioProcessorEditor,
                                            private juce::ChangeListener
{
public:
    explicit ParagonX1AudioProcessorEditor (ParagonX1AudioProcessor&);
    ~ParagonX1AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int defaultWidth = 1200;
    static constexpr int defaultHeight = 675;

private:
    void changeListenerCallback (juce::ChangeBroadcaster*) override;
    void selectBlock (int index);
    juce::String valueText (const char* parameterId) const;

    ParagonX1AudioProcessor& audioProcessor;
    ParagonLookAndFeel lookAndFeel;
    GainColumn inputColumn, outputColumn;
    ModelSlot ampSlot, cabSlot;
    juce::ToggleButton normaliseButton { "NORMALISE" };
    juce::AudioProcessorValueTreeState::ButtonAttachment normaliseAttachment;
    juce::Slider presenceKnob { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxRight };
    juce::Label presenceLabel { {}, "PRESENCE" };
    juce::AudioProcessorValueTreeState::SliderAttachment presenceAttachment;

    // The four effect blocks: a footswitch each, plus the edit panel of the
    // selected one.
    std::array<std::unique_ptr<Footswitch>, 4> footswitches;
    std::array<std::unique_ptr<EffectPanel>, 4> panels;
    int selectedBlock = 0;

    juce::TooltipWindow tooltips { this };
    juce::Rectangle<int> headerArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParagonX1AudioProcessorEditor)
};
