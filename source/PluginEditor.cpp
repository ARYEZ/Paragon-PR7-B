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
      normaliseAttachment (processorToEdit.getParameters(), ParamIDs::ampNormalise, normaliseButton),
      presenceAttachment (processorToEdit.getParameters(), ParamIDs::presence, presenceKnob)
{
    setLookAndFeel (&lookAndFeel);

    ampSlot.onLoad = [this] (const juce::File& file) { audioProcessor.loadAmpModel (file); };
    ampSlot.onClear = [this] { audioProcessor.clearAmpModel(); };
    cabSlot.onLoad = [this] (const juce::File& file) { audioProcessor.loadCabinetIR (file); };
    cabSlot.onClear = [this] { audioProcessor.clearCabinetIR(); };

    normaliseButton.setTooltip ("Level-match models to the same loudness, like the official NAM plugin's "
                                "\"Normalized\" output. Needs loudness data in the model.");

    presenceKnob.setTooltip ("Cut or boost the top end (above about 3.5 kHz) after the cabinet, "
                             "like the presence control on an amp. Double-click to reset.");
    presenceKnob.setDoubleClickReturnValue (true, 0.0);
    presenceKnob.setTextBoxStyle (juce::Slider::TextBoxRight, false, 64, 20);
    presenceKnob.setColour (juce::Slider::textBoxTextColourId, ParagonColours::accent);
    presenceKnob.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    presenceLabel.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    presenceLabel.setJustificationType (juce::Justification::centredRight);

    for (auto* component : std::initializer_list<juce::Component*> { &inputColumn, &outputColumn, &ampSlot, &cabSlot,
                                                                      &normaliseButton, &presenceKnob, &presenceLabel })
        addAndMakeVisible (component);

    // The four footswitches and their edit panels, in signal-chain order.
    auto& state = processorToEdit.getParameters();
    using Group = EffectPanel::Group;

    footswitches[0] = std::make_unique<Footswitch> (1, "COMP / GATE", state, juce::StringArray { ParamIDs::compOn });
    footswitches[0]->describe = [this]
    {
        auto text = valueText (ParamIDs::compThreshold) + "  " + valueText (ParamIDs::compRatio);
        if (audioProcessor.getParameters().getRawParameterValue (ParamIDs::gateThreshold)->load() > effects::CompGate::gateOffDb)
            text << "  + gate";
        return text;
    };
    panels[0] = std::make_unique<EffectPanel> (state, std::vector<Group> {
        { "COMPRESSOR", { { ParamIDs::compThreshold, "THRESHOLD" }, { ParamIDs::compRatio, "RATIO" }, { ParamIDs::compLevel, "LEVEL" } } },
        { "NOISE GATE", { { ParamIDs::gateThreshold, "THRESHOLD" } } } });

    footswitches[1] = std::make_unique<Footswitch> (2, "OD / DISTORTION", state, juce::StringArray { ParamIDs::driveOn });
    footswitches[1]->describe = [this] { return "Drive " + valueText (ParamIDs::driveGain) + "  Tone " + valueText (ParamIDs::driveTone); };
    panels[1] = std::make_unique<EffectPanel> (state, std::vector<Group> {
        { "OVERDRIVE", { { ParamIDs::driveGain, "DRIVE" }, { ParamIDs::driveTone, "TONE" }, { ParamIDs::driveLevel, "LEVEL" } } } });

    footswitches[2] = std::make_unique<Footswitch> (3, "MODULATION", state, juce::StringArray { ParamIDs::modOn });
    footswitches[2]->describe = [this] { return valueText (ParamIDs::modType) + "  " + valueText (ParamIDs::modRate); };
    panels[2] = std::make_unique<EffectPanel> (state, std::vector<Group> {
        { "MODULATION", { { ParamIDs::modType, "TYPE" }, { ParamIDs::modRate, "RATE" }, { ParamIDs::modDepth, "DEPTH" },
                          { ParamIDs::modMix, "MIX" } } } });

    footswitches[3] = std::make_unique<Footswitch> (4, "REVERB / DELAY", state, juce::StringArray { ParamIDs::delayOn, ParamIDs::reverbOn });
    footswitches[3]->describe = [this]
    {
        const auto synced = audioProcessor.getParameters().getRawParameterValue (ParamIDs::delaySync)->load() > 0.5f;
        return "Delay " + valueText (synced ? ParamIDs::delaySync : ParamIDs::delayTime) + "  Reverb " + valueText (ParamIDs::reverbMix);
    };
    panels[3] = std::make_unique<EffectPanel> (state, std::vector<Group> {
        { "DELAY", { { ParamIDs::delayOn, "ON" }, { ParamIDs::delaySync, "SYNC" }, { ParamIDs::delayTime, "TIME", [this] { return audioProcessor.getParameters().getRawParameterValue (ParamIDs::delaySync)->load() < 0.5f; } },
                     { ParamIDs::delayFeedback, "FEEDBACK" }, { ParamIDs::delayTone, "TONE" }, { ParamIDs::delayMix, "MIX" },
                     { ParamIDs::delayPingPong, "PING-PONG" } } },
        { "REVERB", { { ParamIDs::reverbOn, "ON" }, { ParamIDs::reverbSize, "SIZE" }, { ParamIDs::reverbTone, "TONE" },
                      { ParamIDs::reverbMix, "MIX" } } } });

    for (int i = 0; i < (int) footswitches.size(); ++i)
    {
        footswitches[(size_t) i]->onSelect = [this, i] { selectBlock (i); };
        footswitches[(size_t) i]->refresh();
        addAndMakeVisible (*footswitches[(size_t) i]);
        addChildComponent (*panels[(size_t) i]);
    }
    selectBlock (0);

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

juce::String ParagonX1AudioProcessorEditor::valueText (const char* parameterId) const
{
    return audioProcessor.getParameters().getParameter (parameterId)->getCurrentValueAsText();
}

void ParagonX1AudioProcessorEditor::selectBlock (int index)
{
    selectedBlock = index;
    for (size_t i = 0; i < footswitches.size(); ++i)
    {
        footswitches[i]->setSelected ((int) i == index);
        panels[i]->setVisible ((int) i == index);
    }
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

}

void ParagonX1AudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    headerArea = area.removeFromTop (headerHeight);
    area.reduce (margin, margin);

    inputColumn.setBounds (area.removeFromLeft (columnWidth));
    outputColumn.setBounds (area.removeFromRight (columnWidth));
    auto centre = area.reduced (margin, 0);

    // Top row: amp and cabinet side by side, as in the mockup, with the
    // normalise switch under the amp and presence under the cabinet.
    auto slots = centre.removeFromTop (centre.getHeight() * 36 / 100);
    const auto slotWidth = (slots.getWidth() - margin) / 2;
    auto ampColumn = slots.removeFromLeft (slotWidth);
    auto cabColumn = slots.withTrimmedLeft (margin);
    const auto controlRowHeight = 44;
    normaliseButton.setBounds (ampColumn.removeFromBottom (controlRowHeight).withSizeKeepingCentre (130, 24));
    auto presenceRow = cabColumn.removeFromBottom (controlRowHeight).withSizeKeepingCentre (240, controlRowHeight);
    presenceLabel.setBounds (presenceRow.removeFromLeft (80));
    presenceKnob.setBounds (presenceRow);
    ampSlot.setBounds (ampColumn.withTrimmedBottom (4));
    cabSlot.setBounds (cabColumn.withTrimmedBottom (4));

    // Bottom: the four footswitches, then the selected block's edit panel.
    auto effects = centre.withTrimmedTop (margin);
    auto switchRow = effects.removeFromTop (effects.getHeight() * 47 / 100);
    const auto switchWidth = (switchRow.getWidth() - margin * 3) / 4;
    for (auto& footswitch : footswitches)
    {
        footswitch->setBounds (switchRow.removeFromLeft (switchWidth));
        switchRow.removeFromLeft (margin);
    }

    effects.removeFromTop (margin);
    for (auto& panel : panels)
        panel->setBounds (effects);
}
