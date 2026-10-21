#include "PluginEditor.h"

#include "Parameters.h"

namespace
{
    constexpr int headerHeight = 56;
    constexpr int columnWidth = 120;
    constexpr int margin = 12;

    void drawPanel (juce::Graphics& g, juce::Rectangle<float> area)
    {
        g.setColour (ParagonColours::panel);
        g.fillRoundedRectangle (area, 8.0f);
        g.setColour (ParagonColours::panelEdge);
        g.drawRoundedRectangle (area.reduced (0.5f), 8.0f, 1.0f);
    }
}

//==============================================================================
GainColumn::GainColumn (const juce::String& title, juce::AudioProcessorValueTreeState& parameters,
                        const juce::String& parameterId, LevelMeterSource& meterSource)
    : attachment (parameters, parameterId, knob),
      meter (meterSource)
{
    titleLabel.setText (title, juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setFont (juce::FontOptions (15.0f, juce::Font::bold));

    // Set on the knob itself: the editor's look-and-feel is attached after the
    // knob's value box has already been created, so it wouldn't pick these up.
    knob.setColour (juce::Slider::textBoxTextColourId, ParagonColours::accent);
    knob.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    knob.setDoubleClickReturnValue (true, 0.0);   // double-click resets to 0 dB

    addAndMakeVisible (titleLabel);
    addAndMakeVisible (knob);
    addAndMakeVisible (meter);
}

void GainColumn::paint (juce::Graphics& g)
{
    drawPanel (g, getLocalBounds().toFloat());
}

void GainColumn::resized()
{
    auto area = getLocalBounds().reduced (8);
    titleLabel.setBounds (area.removeFromTop (28));
    knob.setBounds (area.removeFromTop (110).reduced (6, 0));
    area.removeFromTop (8);
    meter.setBounds (area.withSizeKeepingCentre (64, area.getHeight()));
}

//==============================================================================
ParagonX1AudioProcessorEditor::ParagonX1AudioProcessorEditor (ParagonX1AudioProcessor& processorToEdit)
    : AudioProcessorEditor (processorToEdit),
      inputColumn ("INPUT", processorToEdit.getParameters(), ParamIDs::inputGain, processorToEdit.getInputMeter()),
      outputColumn ("OUTPUT", processorToEdit.getParameters(), ParamIDs::outputGain, processorToEdit.getOutputMeter())
{
    setLookAndFeel (&lookAndFeel);

    addAndMakeVisible (inputColumn);
    addAndMakeVisible (outputColumn);

    // Keep the mockup's 16:9 shape at any size the user drags to.
    setResizable (true, true);
    setResizeLimits (960, 540, 1920, 1080);
    getConstrainer()->setFixedAspectRatio ((double) defaultWidth / (double) defaultHeight);
    setSize (defaultWidth, defaultHeight);
}

ParagonX1AudioProcessorEditor::~ParagonX1AudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void ParagonX1AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (ParagonColours::background);

    // Header: product name, as in the mockup's top bar.
    auto header = headerArea.reduced (margin, 0);
    g.setFont (juce::FontOptions (24.0f, juce::Font::bold));
    g.setColour (ParagonColours::text);
    const juce::String paragon ("PARAGON ");
    g.drawText (paragon, header, juce::Justification::centredLeft, false);
    const auto nameWidth = juce::GlyphArrangement::getStringWidthInt (g.getCurrentFont(), paragon);
    g.setColour (ParagonColours::accent);
    g.drawText ("X1", header.withTrimmedLeft (nameWidth), juce::Justification::centredLeft, false);

    g.setFont (juce::FontOptions (12.0f));
    g.setColour (ParagonColours::textDim);
    g.drawText ("NAM PLAYER  |  v" JucePlugin_VersionString, header, juce::Justification::centredRight, false);

    g.setColour (ParagonColours::panelEdge);
    g.drawHorizontalLine (headerArea.getBottom() - 1, 0.0f, (float) getWidth());

    // Placeholder for the pedalboard, which arrives in later phases.
    drawPanel (g, centreArea.toFloat());
    g.setColour (ParagonColours::textDim);
    g.setFont (juce::FontOptions (16.0f));
    g.drawText ("Amp, cabinet and effects arrive in the next phases",
                centreArea, juce::Justification::centred, false);
}

void ParagonX1AudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    headerArea = area.removeFromTop (headerHeight);
    area.reduce (margin, margin);

    inputColumn.setBounds (area.removeFromLeft (columnWidth));
    outputColumn.setBounds (area.removeFromRight (columnWidth));
    centreArea = area.reduced (margin, 0);
}
