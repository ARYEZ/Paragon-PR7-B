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
      audioProcessor (processorToEdit),
      inputColumn ("INPUT", processorToEdit.getParameters(), ParamIDs::inputGain, processorToEdit.getInputMeter()),
      outputColumn ("OUTPUT", processorToEdit.getParameters(), ParamIDs::outputGain, processorToEdit.getOutputMeter()),
      ampSlot ("AMP", "*.nam", "Click or drop a .nam model",
               processorToEdit.getParameters(), ParamIDs::ampOn),
      cabSlot ("CABINET", "*.wav;*.aif;*.aiff;*.flac", "Click or drop an IR (.wav)",
               processorToEdit.getParameters(), ParamIDs::cabOn),
      normaliseAttachment (processorToEdit.getParameters(), ParamIDs::ampNormalise, normaliseButton)
{
    setLookAndFeel (&lookAndFeel);

    ampSlot.onLoad = [this] (const juce::File& file) { audioProcessor.loadAmpModel (file); };
    ampSlot.onClear = [this] { audioProcessor.clearAmpModel(); };
    cabSlot.onLoad = [this] (const juce::File& file) { audioProcessor.loadCabinetIR (file); };
    cabSlot.onClear = [this] { audioProcessor.clearCabinetIR(); };

    normaliseButton.setTooltip ("Level-match models to the same loudness, like the official NAM plugin's "
                                "\"Normalized\" output. Needs loudness data in the model.");

    for (auto* component : std::initializer_list<juce::Component*> { &inputColumn, &outputColumn, &ampSlot, &cabSlot, &normaliseButton })
        addAndMakeVisible (component);

    audioProcessor.statusBroadcaster.addChangeListener (this);
    changeListenerCallback (nullptr);

    // Keep the mockup's 16:9 shape at any size the user drags to.
    setResizable (true, true);
    setResizeLimits (960, 540, 1920, 1080);
    getConstrainer()->setFixedAspectRatio ((double) defaultWidth / (double) defaultHeight);
    setSize (defaultWidth, defaultHeight);
}

ParagonX1AudioProcessorEditor::~ParagonX1AudioProcessorEditor()
{
    audioProcessor.statusBroadcaster.removeChangeListener (this);
    setLookAndFeel (nullptr);
}

void ParagonX1AudioProcessorEditor::changeListenerCallback (juce::ChangeBroadcaster*)
{
    ampSlot.setStatus (audioProcessor.getAmpStatus());
    cabSlot.setStatus (audioProcessor.getCabStatus());
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

    // Placeholder for the effect footswitches, which arrive in Phase 3.
    drawPanel (g, effectsArea.toFloat());
    g.setColour (ParagonColours::textDim);
    g.setFont (juce::FontOptions (16.0f));
    g.drawText ("Comp / gate, drive, modulation, delay and reverb arrive in the next phase",
                effectsArea, juce::Justification::centred, false);
}

void ParagonX1AudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    headerArea = area.removeFromTop (headerHeight);
    area.reduce (margin, margin);

    inputColumn.setBounds (area.removeFromLeft (columnWidth));
    outputColumn.setBounds (area.removeFromRight (columnWidth));
    auto centre = area.reduced (margin, 0);

    // Top row: amp and cabinet side by side, as in the mockup.
    auto slots = centre.removeFromTop (centre.getHeight() * 45 / 100);
    const auto slotWidth = (slots.getWidth() - margin) / 2;
    auto ampColumn = slots.removeFromLeft (slotWidth);
    normaliseButton.setBounds (ampColumn.removeFromBottom (28).withSizeKeepingCentre (130, 24));
    ampColumn.removeFromBottom (4);
    ampSlot.setBounds (ampColumn);
    cabSlot.setBounds (slots.withTrimmedLeft (margin).withTrimmedBottom (32));

    effectsArea = centre.withTrimmedTop (margin);
}
