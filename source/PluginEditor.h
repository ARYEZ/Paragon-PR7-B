#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "PluginProcessor.h"
#include "ui/LevelMeter.h"
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

class ParagonX1AudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit ParagonX1AudioProcessorEditor (ParagonX1AudioProcessor&);
    ~ParagonX1AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int defaultWidth = 1200;
    static constexpr int defaultHeight = 675;

private:
    ParagonLookAndFeel lookAndFeel;
    GainColumn inputColumn, outputColumn;
    juce::Rectangle<int> headerArea, centreArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParagonX1AudioProcessorEditor)
};
