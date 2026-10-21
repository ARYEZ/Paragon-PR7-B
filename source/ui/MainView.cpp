#include "MainView.h"

#include "Parameters.h"

namespace
{
    // Layout on the 1600 x 900 canvas, following the mockup's proportions.
    constexpr int headerHeight = 60;
    constexpr int margin = 12;
    const juce::Rectangle<int> pedalArea { 134, 66, 1332, 480 };
    const juce::Rectangle<int> dockBounds { 12, 556, 1576, 250 };
    const juce::Rectangle<int> chainBounds { 12, 816, 1576, 74 };
    constexpr int currentRigWidth = 400;

    // Positions on the pedal, relative to its top-left corner.
    const juce::Rectangle<int> badgeArea { 50, 40, 280, 110 };
    const juce::Rectangle<int> displayArea { 340, 36, 450, 104 };
    const juce::Rectangle<int> ampWindow { 40, 182, 410, 130 };
    const juce::Rectangle<int> cabWindow { 462, 182, 410, 130 };
    const juce::Rectangle<int> rigWindow { 884, 182, 408, 130 };
    constexpr int topDividerY = 170, bottomDividerY = 324;
    const juce::Rectangle<int> footswitchRow { 40, 330, 1252, 136 };

    void drawDarkPanel (juce::Graphics& g, juce::Rectangle<float> area)
    {
        g.setColour (ParagonColours::panel);
        g.fillRoundedRectangle (area, 8.0f);
        g.setColour (ParagonColours::panelEdge);
        g.drawRoundedRectangle (area.reduced (0.5f), 8.0f, 1.0f);
    }

    void styleInkLabel (juce::Label& label, float size)
    {
        label.setFont (juce::FontOptions (size, juce::Font::bold));
        label.setColour (juce::Label::textColourId, ParagonColours::ink);
        label.setJustificationType (juce::Justification::centred);
        label.setInterceptsMouseClicks (false, false);
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
    drawDarkPanel (g, getLocalBounds().toFloat());
}

void GainColumn::resized()
{
    auto area = getLocalBounds().reduced (8);
    titleLabel.setBounds (area.removeFromTop (28));
    knob.setBounds (area.removeFromTop (100).reduced (6, 0));
    area.removeFromTop (8);
    meter.setBounds (area.withSizeKeepingCentre (64, area.getHeight()));
}

//==============================================================================
MainView::MainView (ParagonX1AudioProcessor& processorToShow)
    : audioProcessor (processorToShow),
      inputColumn ("INPUT", processorToShow.getParameters(), ParamIDs::inputGain, processorToShow.getInputMeter()),
      outputColumn ("OUTPUT", processorToShow.getParameters(), ParamIDs::outputGain, processorToShow.getOutputMeter()),
      bypassAttachment (processorToShow.getParameters(), ParamIDs::bypass, bypassButton),
      rigLevelAttachment (processorToShow.getParameters(), ParamIDs::rigLevel, rigLevelKnob),
      presenceAttachment (processorToShow.getParameters(), ParamIDs::presence, presenceKnob),
      ampSlot ("AMP", Icons::Type::amp, ParagonColours::amp, "*.nam", "Load / Edit NAM Model",
               processorToShow.getParameters(), ParamIDs::ampOn),
      cabSlot ("CABINET", Icons::Type::cabinet, ParagonColours::text, "*.wav;*.aif;*.aiff;*.flac", "Load / Edit IR",
               processorToShow.getParameters(), ParamIDs::cabOn),
      normaliseAttachment (processorToShow.getParameters(), ParamIDs::ampNormalise, normaliseButton),
      currentRigPanel (processorToShow),
      signalChain (processorToShow.getParameters(), {
          { "COMP / GATE", Icons::Type::comp, ParagonColours::comp, ParamIDs::compOn },
          { "OD / DIST", Icons::Type::drive, ParagonColours::drive, ParamIDs::driveOn },
          { "NAMP", Icons::Type::amp, ParagonColours::amp, ParamIDs::ampOn },
          { "CABINET IR", Icons::Type::cabinet, ParagonColours::cab, ParamIDs::cabOn },
          { "MODULATION", Icons::Type::modulation, ParagonColours::modulation, ParamIDs::modOn },
          { "DELAY", Icons::Type::delay, ParagonColours::delay, ParamIDs::delayOn },
          { "REVERB", Icons::Type::reverb, ParagonColours::reverb, ParamIDs::reverbOn } }),
      rigBrowser (processorToShow)
{
    auto& state = processorToShow.getParameters();

    //==========================================================================
    // Header
    for (auto* tab : { &processorTab, &presetsTab })
    {
        tab->setClickingTogglesState (true);
        tab->setRadioGroupId (100);
        tab->setColour (juce::TextButton::buttonOnColourId, ParagonColours::accentDim);
        tab->setColour (juce::TextButton::textColourOnId, ParagonColours::accent);
        addAndMakeVisible (*tab);
    }
    processorTab.setToggleState (true, juce::dontSendNotification);
    processorTab.setComponentID ("processorTab");
    presetsTab.setComponentID ("presetsTab");
    processorTab.onClick = [this] { showPage (false); };
    presetsTab.onClick = [this] { showPage (true); };

    previousRig.setTooltip ("Previous rig");
    nextRig.setTooltip ("Next rig");
    rigNameButton.setTooltip ("The current rig. * means it has changed since it was loaded or saved. Click to browse rigs.");
    saveButton.setTooltip ("Save the current sound over this rig (or as a new rig if it's a factory rig)");
    previousRig.onClick = [this] { audioProcessor.stepRig (-1); };
    nextRig.onClick = [this] { audioProcessor.stepRig (1); };
    rigNameButton.onClick = [this] { showPage (true); };
    rigNameButton.setColour (juce::TextButton::textColourOffId, ParagonColours::accent);
    saveButton.onClick = [this] { saveRig(); };

    tempoLabel.setFont (juce::FontOptions (14.0f));
    tempoLabel.setColour (juce::Label::textColourId, ParagonColours::text);
    tempoLabel.setColour (juce::Label::outlineColourId, ParagonColours::panelEdge);
    tempoLabel.setJustificationType (juce::Justification::centred);
    tempoLabel.setTooltip ("Your DAW's tempo, used by the delay when it's synced");

    for (auto* component : std::initializer_list<juce::Component*> { &previousRig, &nextRig, &rigNameButton, &saveButton, &tempoLabel,
                                                                      &inputColumn, &outputColumn })
        addAndMakeVisible (component);

    //==========================================================================
    // The pedal unit
    addAndMakeVisible (pedal);

    display.onClick = [this] { showPage (true); };
    rigUp.onClick = [this] { audioProcessor.stepRig (-1); };
    rigDown.onClick = [this] { audioProcessor.stepRig (1); };

    for (auto* button : { &pedalSave, &pedalSaveAs, &bypassButton })
        button->getProperties().set (ParagonLookAndFeel::pedalButtonProperty, true);
    pedalSave.onClick = [this] { saveRig(); };
    pedalSaveAs.onClick = [this] { rigBrowser.promptSaveAs(); };
    bypassButton.setClickingTogglesState (true);
    bypassButton.setTooltip ("Bypass the whole plugin (the same as your DAW's bypass button)");

    for (auto* knob : { &rigLevelKnob, &presenceKnob })
    {
        knob->getProperties().set (ParagonLookAndFeel::pedalKnobProperty, true);
        knob->setDoubleClickReturnValue (true, 0.0);
        knob->setPopupDisplayEnabled (true, true, this);   // shows the value while dragging
    }
    rigLevelKnob.setTooltip ("This rig's volume, saved with the rig: use it to balance rigs. "
                             "The OUTPUT knob on the right is the overall volume.");
    presenceKnob.setTooltip ("Cut or boost the top end (above about 3.5 kHz) after the cabinet, like an amp's presence control.");

    styleInkLabel (bypassLabel, 11.0f);
    styleInkLabel (rigLevelLabel, 13.0f);
    styleInkLabel (presenceLabel, 13.0f);

    ampSlot.onLoad = [this] (const juce::File& file) { audioProcessor.loadAmpModel (file); };
    ampSlot.onClear = [this] { audioProcessor.clearAmpModel(); };
    ampSlot.onTitleClicked = [this] { selectBlock (ampBlock); };
    cabSlot.onLoad = [this] (const juce::File& file) { audioProcessor.loadCabinetIR (file); };
    cabSlot.onClear = [this] { audioProcessor.clearCabinetIR(); };
    cabSlot.onTitleClicked = [this] { selectBlock (cabBlock); };
    rigSlot.onStep = [this] (int direction) { audioProcessor.stepRig (direction); };
    rigSlot.onOpen = [this] { showPage (true); };

    normaliseButton.setColour (juce::ToggleButton::textColourId, ParagonColours::ink);
    normaliseButton.setTooltip ("Level-match models to the same loudness, like the official NAM plugin's "
                                "\"Normalized\" output. Needs loudness data in the model.");

    // The four footswitches, in signal-chain order. Footswitch 4 switches
    // delay and reverb together; clicking its name shows the delay's knobs.
    footswitches[0] = std::make_unique<Footswitch> (1, "COMP / GATE", state, juce::StringArray { ParamIDs::compOn });
    footswitches[0]->describe = [this]
    {
        auto text = valueText (ParamIDs::compThreshold) + "  " + valueText (ParamIDs::compRatio);
        if (audioProcessor.getParameters().getRawParameterValue (ParamIDs::gateThreshold)->load() > effects::CompGate::gateOffDb)
            text << "  + gate";
        return text;
    };
    footswitches[1] = std::make_unique<Footswitch> (2, "OD / DISTORTION", state, juce::StringArray { ParamIDs::driveOn });
    footswitches[1]->describe = [this] { return "Drive " + valueText (ParamIDs::driveGain) + "  Tone " + valueText (ParamIDs::driveTone); };
    footswitches[2] = std::make_unique<Footswitch> (3, "MODULATION", state, juce::StringArray { ParamIDs::modOn });
    footswitches[2]->describe = [this] { return valueText (ParamIDs::modType) + "  " + valueText (ParamIDs::modRate); };
    footswitches[3] = std::make_unique<Footswitch> (4, "REVERB / DELAY", state, juce::StringArray { ParamIDs::delayOn, ParamIDs::reverbOn });
    footswitches[3]->describe = [this]
    {
        const auto synced = audioProcessor.getParameters().getRawParameterValue (ParamIDs::delaySync)->load() > 0.5f;
        return "Delay " + valueText (synced ? ParamIDs::delaySync : ParamIDs::delayTime) + "  Reverb " + valueText (ParamIDs::reverbMix);
    };
    const int footswitchBlocks[] = { compBlock, driveBlock, modBlock, delayBlock };
    for (size_t i = 0; i < footswitches.size(); ++i)
    {
        footswitches[i]->onSelect = [this, block = footswitchBlocks[i]] { selectBlock (block); };
        footswitches[i]->refresh();
    }

    processorPage = { &pedal, &display, &rigUp, &rigDown, &pedalSave, &pedalSaveAs, &bypassButton, &bypassLabel,
                      &rigLevelKnob, &presenceKnob, &rigLevelLabel, &presenceLabel,
                      &ampSlot, &cabSlot, &rigSlot, &normaliseButton, &currentRigPanel };
    for (auto& footswitch : footswitches)
        processorPage.push_back (footswitch.get());
    for (auto* component : processorPage)
        addAndMakeVisible (component);

    //==========================================================================
    // Edit panels, one per block. Each starts with the block's on switch.
    using Group = EffectPanel::Group;
    panels[compBlock] = std::make_unique<EffectPanel> (state, std::vector<Group> {
        { "COMPRESSOR", { { ParamIDs::compOn, "ON" }, { ParamIDs::compThreshold, "THRESHOLD" }, { ParamIDs::compRatio, "RATIO" },
                          { ParamIDs::compLevel, "LEVEL" } } },
        { "NOISE GATE", { { ParamIDs::gateThreshold, "THRESHOLD" } } } });
    panels[driveBlock] = std::make_unique<EffectPanel> (state, std::vector<Group> {
        { "OVERDRIVE", { { ParamIDs::driveOn, "ON" }, { ParamIDs::driveGain, "DRIVE" }, { ParamIDs::driveTone, "TONE" },
                         { ParamIDs::driveLevel, "LEVEL" } } } });
    panels[ampBlock] = std::make_unique<EffectPanel> (state, std::vector<Group> {
        { "AMP (NAM)", { { ParamIDs::ampOn, "ON" }, { ParamIDs::ampNormalise, "NORMALISE" }, { ParamIDs::presence, "PRESENCE" },
                         { ParamIDs::rigLevel, "OUTPUT LEVEL" } } } });
    panels[cabBlock] = std::make_unique<EffectPanel> (state, std::vector<Group> {
        { "CABINET (IR)", { { ParamIDs::cabOn, "ON" }, { ParamIDs::presence, "PRESENCE" } } } });
    panels[modBlock] = std::make_unique<EffectPanel> (state, std::vector<Group> {
        { "MODULATION", { { ParamIDs::modOn, "ON" }, { ParamIDs::modType, "TYPE" }, { ParamIDs::modRate, "RATE" },
                          { ParamIDs::modDepth, "DEPTH" }, { ParamIDs::modMix, "MIX" } } } });
    panels[delayBlock] = std::make_unique<EffectPanel> (state, std::vector<Group> {
        { "DELAY", { { ParamIDs::delayOn, "ON" }, { ParamIDs::delaySync, "SYNC" },
                     { ParamIDs::delayTime, "TIME", [this] { return audioProcessor.getParameters().getRawParameterValue (ParamIDs::delaySync)->load() < 0.5f; } },
                     { ParamIDs::delayFeedback, "FEEDBACK" }, { ParamIDs::delayTone, "TONE" }, { ParamIDs::delayMix, "MIX" },
                     { ParamIDs::delayPingPong, "PING-PONG" } } } });
    panels[reverbBlock] = std::make_unique<EffectPanel> (state, std::vector<Group> {
        { "REVERB", { { ParamIDs::reverbOn, "ON" }, { ParamIDs::reverbSize, "SIZE" }, { ParamIDs::reverbTone, "TONE" },
                      { ParamIDs::reverbMix, "MIX" } } } });
    for (auto& panel : panels)
        addChildComponent (*panel);

    currentRigPanel.onSave = [this] { saveRig(); };
    currentRigPanel.onSaveAs = [this] { rigBrowser.promptSaveAs(); };
    currentRigPanel.onBrowse = [this] { showPage (true); };

    signalChain.onSelect = [this] (int block) { selectBlock (block); };
    addAndMakeVisible (signalChain);
    addChildComponent (rigBrowser);

    selectBlock (compBlock);

    audioProcessor.rigBroadcaster.addChangeListener (this);
    audioProcessor.statusBroadcaster.addChangeListener (this);
    changeListenerCallback (nullptr);
    startTimerHz (5);   // the modified marker and tempo can change from the audio thread, so poll them
    timerCallback();
}

MainView::~MainView()
{
    audioProcessor.statusBroadcaster.removeChangeListener (this);
    audioProcessor.rigBroadcaster.removeChangeListener (this);
}

juce::String MainView::valueText (const char* parameterId) const
{
    return audioProcessor.getParameters().getParameter (parameterId)->getCurrentValueAsText();
}

void MainView::selectBlock (int block)
{
    selectedBlock = block;
    for (size_t i = 0; i < panels.size(); ++i)
        panels[i]->setVisible ((int) i == block && ! showingPresets);

    const int footswitchBlocks[] = { compBlock, driveBlock, modBlock, delayBlock };
    for (size_t i = 0; i < footswitches.size(); ++i)
        footswitches[i]->setSelected (footswitchBlocks[i] == block || (i == 3 && block == reverbBlock));

    signalChain.setSelected (block);
}

void MainView::showPage (bool presets)
{
    showingPresets = presets;
    (presets ? presetsTab : processorTab).setToggleState (true, juce::dontSendNotification);
    for (auto* component : processorPage)
        component->setVisible (! presets);
    rigBrowser.setVisible (presets);
    selectBlock (selectedBlock);
}

void MainView::refreshRig()
{
    const auto rig = audioProcessor.getCurrentRig();
    const auto* entry = audioProcessor.findCurrentRigEntry();
    const auto modified = audioProcessor.isRigModified();
    const auto bypassed = audioProcessor.getParameters().getRawParameterValue (ParamIDs::bypass)->load() > 0.5f;

    juce::String headerText;
    if (entry != nullptr)
        headerText << entry->rig.getSlotName() << "   ";
    headerText << rig.name << (modified ? " *" : "");
    if (rigNameButton.getButtonText() != headerText)
        rigNameButton.setButtonText (headerText);

    display.setContents ({ entry != nullptr ? entry->rig.number : 0, rig.name,
                           entry != nullptr ? Rig::bankName (entry->rig.bank) : juce::String(), modified, bypassed });
    rigSlot.setRig (entry != nullptr ? entry->rig.getSlotName() : juce::String(), rig.name, modified);
    currentRigPanel.refresh();
}

void MainView::timerCallback()
{
    refreshRig();

    const auto tempo = juce::String (audioProcessor.getHostBpm(), 1) + " BPM";
    if (tempoLabel.getText() != tempo)
        tempoLabel.setText (tempo, juce::dontSendNotification);
}

void MainView::saveRig()
{
    // Factory rigs and never-saved sounds need a name first.
    if (! audioProcessor.saveCurrentRig())
        rigBrowser.promptSaveAs();
    refreshRig();
}

void MainView::changeListenerCallback (juce::ChangeBroadcaster*)
{
    refreshRig();
    ampSlot.setStatus (audioProcessor.getAmpStatus());
    cabSlot.setStatus (audioProcessor.getCabStatus());
}

void MainView::paint (juce::Graphics& g)
{
    g.fillAll (ParagonColours::background);

    // Header: emblem and name, as in the mockup's top bar.
    auto header = headerArea.reduced (16, 0).toFloat();
    auto emblem = header.removeFromLeft (40.0f).withSizeKeepingCentre (34.0f, 30.0f);
    juce::Path triangle;
    triangle.addTriangle (emblem.getTopLeft(), emblem.getTopRight(), { emblem.getCentreX(), emblem.getBottom() });
    g.setColour (ParagonColours::glow.withAlpha (0.3f));
    g.strokePath (triangle, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (ParagonColours::glow);
    g.strokePath (triangle, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    triangle.applyTransform (juce::AffineTransform::scale (0.5f, 0.5f, emblem.getCentreX(), emblem.getCentreY() - 3.0f));
    g.strokePath (triangle, juce::PathStrokeType (1.5f));

    header.removeFromLeft (10.0f);
    g.setFont (juce::FontOptions (22.0f));
    g.setColour (ParagonColours::text);
    g.drawText ("PARAGON", header, juce::Justification::centredLeft, false);
    const auto nameWidth = (float) juce::GlyphArrangement::getStringWidthInt (juce::FontOptions (22.0f), "PARAGON ");
    g.setColour (ParagonColours::glow);
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText ("X1", header.withTrimmedLeft (nameWidth), juce::Justification::centredLeft, false);

    g.setFont (juce::FontOptions (11.0f));
    g.setColour (ParagonColours::textDim);
    g.drawText ("v" JucePlugin_VersionString, headerArea.reduced (16, 0), juce::Justification::centredRight, false);

    g.setColour (ParagonColours::panelEdge);
    g.drawHorizontalLine (headerArea.getBottom() - 1, 0.0f, (float) getWidth());
}

void MainView::resized()
{
    headerArea = getLocalBounds().removeFromTop (headerHeight);

    // Header row
    auto header = headerArea.reduced (margin, 12);
    header.removeFromLeft (200);
    processorTab.setBounds (header.removeFromLeft (130));
    header.removeFromLeft (4);
    presetsTab.setBounds (header.removeFromLeft (130));

    auto right = headerArea.reduced (margin, 12);
    right.removeFromRight (50);   // version text
    saveButton.setBounds (right.removeFromRight (80));
    right.removeFromRight (10);
    tempoLabel.setBounds (right.removeFromRight (110));

    auto rigNav = headerArea.withSizeKeepingCentre (440, headerHeight - 24).translated (60, 0);
    previousRig.setBounds (rigNav.removeFromLeft (34));
    nextRig.setBounds (rigNav.removeFromRight (34));
    rigNameButton.setBounds (rigNav.reduced (6, 0));

    // Side strips and the pedal
    inputColumn.setBounds ({ margin, pedalArea.getY(), pedalArea.getX() - margin * 2, pedalArea.getHeight() });
    outputColumn.setBounds ({ pedalArea.getRight() + margin, pedalArea.getY(), width - pedalArea.getRight() - margin * 2, pedalArea.getHeight() });
    pedal.setBounds (pedalArea);

    const auto onPedal = [] (juce::Rectangle<int> r) { return r.translated (pedalArea.getX(), pedalArea.getY()); };

    display.setBounds (onPedal (displayArea));
    rigUp.setBounds (onPedal ({ displayArea.getRight() + 18, displayArea.getY() + 14, 26, 26 }));
    rigDown.setBounds (onPedal ({ displayArea.getRight() + 18, displayArea.getBottom() - 40, 26, 26 }));
    pedalSave.setBounds (onPedal ({ displayArea.getX() + 70, displayArea.getBottom() + 8, 90, 24 }));
    pedalSaveAs.setBounds (onPedal ({ displayArea.getRight() - 160, displayArea.getBottom() + 8, 110, 24 }));

    bypassLabel.setBounds (onPedal ({ 860, 56, 80, 16 }));
    bypassButton.setBounds (onPedal ({ 870, 76, 60, 26 }));

    rigLevelKnob.setBounds (onPedal ({ 990, 30, 104, 104 }));
    rigLevelLabel.setBounds (onPedal ({ 962, 136, 160, 18 }));
    presenceKnob.setBounds (onPedal ({ 1158, 30, 104, 104 }));
    presenceLabel.setBounds (onPedal ({ 1130, 136, 160, 18 }));

    ampSlot.setBounds (onPedal (ampWindow));
    cabSlot.setBounds (onPedal (cabWindow));
    rigSlot.setBounds (onPedal (rigWindow));
    normaliseButton.setBounds (onPedal ({ ampWindow.getX() + 10, ampWindow.getY() + 8, 120, 22 }));

    auto switches = onPedal (footswitchRow);
    const auto switchWidth = switches.getWidth() / 4;
    for (auto& footswitch : footswitches)
        footswitch->setBounds (switches.removeFromLeft (switchWidth));

    pedal.setInsets ({ ampWindow, cabWindow, rigWindow }, { topDividerY, bottomDividerY }, badgeArea);

    // Dock: the selected block's knobs, and the current rig
    dockArea = dockBounds;
    auto dock = dockArea;
    currentRigPanel.setBounds (dock.removeFromRight (currentRigWidth));
    dock.removeFromRight (margin);
    for (auto& panel : panels)
        panel->setBounds (dock);

    signalChain.setBounds (chainBounds);

    // PRESETS page: covers the pedal and the dock
    rigBrowser.setBounds (pedalArea.getUnion (dockBounds).withX (pedalArea.getX()).withRight (pedalArea.getRight()));
}
