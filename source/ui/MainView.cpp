#include "MainView.h"

#include "Parameters.h"

namespace
{
    // Layout on the 1600 x 900 canvas, following the mockup's proportions.
    constexpr int headerHeight = 60;
    constexpr int margin = 12;
    const juce::Rectangle<int> pedalArea { 134, 66, 1332, 480 };   // the unit and its expression pedal
    constexpr int unitWidth = 1162;                                  // the silver unit; the pedal fills the rest
    const juce::Rectangle<int> dockBounds { 12, 556, 1576, 250 };
    const juce::Rectangle<int> chainBounds { 12, 816, 1576, 74 };
    constexpr int currentRigWidth = 400, expressionPanelWidth = 300;

    // Positions on the unit, relative to its top-left corner.
    const juce::Rectangle<int> badgeArea { 40, 40, 236, 100 };
    const juce::Rectangle<int> displayArea { 282, 36, 404, 104 };
    const juce::Rectangle<int> ampWindow { 36, 182, 292, 130 };
    const juce::Rectangle<int> cabWindow { 338, 182, 292, 130 };
    const juce::Rectangle<int> rigWindow { 640, 182, 292, 130 };
    const juce::Rectangle<int> assignArea { 944, 178, 190, 140 };
    constexpr int topDividerY = 170, bottomDividerY = 324;
    const juce::Rectangle<int> footswitchRow { 36, 330, 1090, 136 };

    // The button cluster: three columns, two rows, as in the mockup.
    constexpr int clusterX = 742, clusterColumn = 62, clusterButtonWidth = 48;
    constexpr int clusterTopY = 44, clusterRowGap = 50;

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
      tunerAttachment (processorToShow.getParameters(), ParamIDs::tunerOn, tunerPedal),
      muteAttachment (processorToShow.getParameters(), ParamIDs::tunerMute, mutePedal),
      rigLevelAttachment (processorToShow.getParameters(), ParamIDs::rigLevel, rigLevelKnob),
      presenceAttachment (processorToShow.getParameters(), ParamIDs::presence, presenceKnob),
      ampSlot ("AMP", Icons::Type::amp, ParagonColours::amp, "*.nam", "Load / Edit NAM Model",
               processorToShow.getParameters(), ParamIDs::ampOn),
      cabSlot ("CABINET", Icons::Type::cabinet, ParagonColours::text, "*.wav;*.aif;*.aiff;*.flac", "Load / Edit IR",
               processorToShow.getParameters(), ParamIDs::cabOn),
      normaliseAttachment (processorToShow.getParameters(), ParamIDs::ampNormalise, normaliseButton),
      pedalAssign (processorToShow.getParameters(), ExpressionAssign::Style::pedal),
      expressionPedal (processorToShow),
      expressionPanel (processorToShow.getParameters()),
      tunerReader (processorToShow),
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

    // Tempo: shows the DAW's tempo, or the tapped one (in orange) once TAP
    // has been used. Clicking a tapped tempo goes back to following the DAW.
    tempoButton.onClick = [this] { audioProcessor.clearTappedTempo(); refreshPerformance(); };
    tapButton.setOutlined (true);
    tapButton.onClick = [this] { if (! headerTapHold.consumeHold()) tap(); };
    tapButton.addMouseListener (&headerTapHold, false);
    headerTapHold.onHold = [this] { toggleTuner(); };
    undoButton.onClick = [this] { audioProcessor.undo(); };
    redoButton.onClick = [this] { audioProcessor.redo(); };

    for (auto* component : std::initializer_list<juce::Component*> { &previousRig, &nextRig, &rigNameButton, &saveButton, &tempoButton,
                                                                      &tapButton, &undoButton, &redoButton, &inputColumn, &outputColumn })
        addAndMakeVisible (component);

    //==========================================================================
    // The pedal unit
    addAndMakeVisible (pedal);

    display.onClick = [this] { showPage (true); };
    rigUp.onClick = [this] { audioProcessor.stepRig (-1); };
    rigDown.onClick = [this] { audioProcessor.stepRig (1); };

    for (auto* button : { &pedalSave, &pedalSaveAs, &bypassButton, &undoPedal, &redoPedal, &tunerPedal, &mutePedal, &tapPedal })
        button->getProperties().set (ParagonLookAndFeel::pedalButtonProperty, true);
    pedalSave.onClick = [this] { saveRig(); };
    pedalSaveAs.onClick = [this] { rigBrowser.promptSaveAs(); };
    bypassButton.setClickingTogglesState (true);
    bypassButton.setTooltip ("Bypass the whole plugin (the same as your DAW's bypass button)");

    undoPedal.setTooltip ("Undo the last change (Ctrl+Z)");
    redoPedal.setTooltip ("Redo (Ctrl+Shift+Z)");
    undoPedal.onClick = [this] { audioProcessor.undo(); };
    redoPedal.onClick = [this] { audioProcessor.redo(); };
    tunerPedal.setClickingTogglesState (true);
    tunerPedal.setTooltip ("Tuner: shows the note you're playing on the screen");
    mutePedal.setClickingTogglesState (true);
    mutePedal.setTooltip ("Mute the output while the tuner is on, so you can tune silently");
    tapPedal.setTooltip ("Tap the tempo for the delay (its light flashes in time). Hold to open the tuner.");
    tapPedal.onClick = [this] { if (! pedalTapHold.consumeHold()) tap(); };
    tapPedal.addMouseListener (&pedalTapHold, false);
    pedalTapHold.onHold = [this] { toggleTuner(); };

    for (auto* label : { &undoLabel, &redoLabel, &tunerLabel, &muteLabel, &tapLabel })
        styleInkLabel (*label, 11.0f);
    styleInkLabel (holdLabel, 9.0f);
    holdLabel.setColour (juce::Label::textColourId, ParagonColours::glow.darker (0.2f));

    tunerReader.onReading = [this] (const TunerReader::Reading& reading)
    {
        tunerState.hasNote = reading.hasNote;
        tunerState.note = reading.note.name;
        tunerState.octave = reading.note.octave;
        tunerState.cents = reading.note.cents;
        tunerState.frequency = reading.frequency;
        refreshRig();
    };

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
                      &undoPedal, &redoPedal, &tunerPedal, &mutePedal, &tapPedal,
                      &undoLabel, &redoLabel, &tunerLabel, &muteLabel, &tapLabel, &holdLabel,
                      &rigLevelKnob, &presenceKnob, &rigLevelLabel, &presenceLabel,
                      &ampSlot, &cabSlot, &rigSlot, &normaliseButton, &pedalAssign, &expressionPedal,
                      &currentRigPanel, &expressionPanel };
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

    // The modified marker, tempo and undo state can change from the audio
    // thread or the host, so poll them. 30 Hz for the TAP light's flash.
    startTimerHz (30);
    timerCallback();

    setWantsKeyboardFocus (true);
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
                           entry != nullptr ? Rig::bankName (entry->rig.bank) : juce::String(), modified, bypassed, tunerState });
    rigSlot.setRig (entry != nullptr ? entry->rig.getSlotName() : juce::String(), rig.name, modified);
    currentRigPanel.refresh();
}

void MainView::timerCallback()
{
    // The rig details change rarely: 5 times a second is plenty for them.
    if (ticks++ % 6 == 0)
        refreshRig();

    refreshPerformance();
}

void MainView::refreshPerformance()
{
    // Tempo: the tapped tempo in orange, or the DAW's.
    const auto tapped = audioProcessor.getTappedBpm() > 0.0;
    const auto tempo = juce::String (audioProcessor.getTempoBpm(), 1) + " BPM";
    if (tempoButton.getButtonText() != tempo)
        tempoButton.setButtonText (tempo);
    tempoButton.setColour (juce::TextButton::textColourOffId, tapped ? ParagonColours::accent : ParagonColours::text);
    tempoButton.setTooltip (tapped ? "Tapped tempo, used by the delay when it's synced. Click to follow your DAW's tempo again."
                                   : "Your DAW's tempo, used by the delay when it's synced. Use TAP to set your own.");

    // TAP's light flashes on each beat, like a hardware unit's.
    const auto beatMs = 60000.0 / audioProcessor.getTempoBpm();
    tapPedal.setToggleState (std::fmod (juce::Time::getMillisecondCounterHiRes(), beatMs) < beatMs * 0.25,
                             juce::dontSendNotification);

    const auto canUndo = audioProcessor.canUndo(), canRedo = audioProcessor.canRedo();
    undoButton.setEnabled (canUndo);
    redoButton.setEnabled (canRedo);
    undoPedal.setToggleState (canUndo, juce::dontSendNotification);   // lit while there's something to undo
    redoPedal.setToggleState (canRedo, juce::dontSendNotification);

    // The tuner runs only while it's switched on.
    auto& parameters = audioProcessor.getParameters();
    const auto tunerOn = parameters.getRawParameterValue (ParamIDs::tunerOn)->load() > 0.5f;
    const auto muted = parameters.getRawParameterValue (ParamIDs::tunerMute)->load() > 0.5f;
    tunerReader.setActive (tunerOn);
    if (tunerState.active != tunerOn || tunerState.muted != muted)
    {
        tunerState.active = tunerOn;
        tunerState.muted = muted;
        if (! tunerOn)
            tunerState.hasNote = false;
        refreshRig();
    }
}

void MainView::tap()
{
    audioProcessor.tapTempo();
    refreshPerformance();
}

void MainView::toggleTuner()
{
    if (auto* tuner = audioProcessor.getParameters().getParameter (ParamIDs::tunerOn))
    {
        tuner->beginChangeGesture();
        tuner->setValueNotifyingHost (tuner->getValue() > 0.5f ? 0.0f : 1.0f);
        tuner->endChangeGesture();
    }
    refreshPerformance();
}

bool MainView::keyPressed (const juce::KeyPress& key)
{
    // Ctrl+Z / Ctrl+Shift+Z / Ctrl+Y (Cmd on a Mac). DAWs often keep the
    // keyboard for themselves, so these work when the plugin window has focus.
    const auto modifiers = key.getModifiers();
    if (! modifiers.isCommandDown())
        return false;

    const auto keyCode = juce::CharacterFunctions::toUpperCase ((juce::juce_wchar) key.getKeyCode());
    if (keyCode == 'Z')
    {
        if (modifiers.isShiftDown())
            audioProcessor.redo();
        else
            audioProcessor.undo();
        return true;
    }
    if (keyCode == 'Y')
    {
        audioProcessor.redo();
        return true;
    }
    return false;
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
    auto header = headerArea.reduced (margin, 10);
    header.removeFromLeft (190);
    processorTab.setBounds (header.removeFromLeft (156));
    header.removeFromLeft (4);
    presetsTab.setBounds (header.removeFromLeft (140));

    auto right = headerArea.reduced (margin, 12);
    right.removeFromRight (50);   // version text
    saveButton.setBounds (right.removeFromRight (74));
    right.removeFromRight (14);
    redoButton.setBounds (right.removeFromRight (40));
    undoButton.setBounds (right.removeFromRight (40));
    right.removeFromRight (14);
    tapButton.setBounds (right.removeFromRight (78));
    right.removeFromRight (6);
    tempoButton.setBounds (right.removeFromRight (104));

    auto rigNav = headerArea.withSizeKeepingCentre (400, headerHeight - 24).translated (20, 0);
    previousRig.setBounds (rigNav.removeFromLeft (34));
    nextRig.setBounds (rigNav.removeFromRight (34));
    rigNameButton.setBounds (rigNav.reduced (6, 0));

    // Side strips and the pedal
    inputColumn.setBounds ({ margin, pedalArea.getY(), pedalArea.getX() - margin * 2, pedalArea.getHeight() });
    outputColumn.setBounds ({ pedalArea.getRight() + margin, pedalArea.getY(), width - pedalArea.getRight() - margin * 2, pedalArea.getHeight() });
    pedal.setBounds (pedalArea.withWidth (unitWidth));
    expressionPedal.setBounds (pedalArea.withTrimmedLeft (unitWidth - 4));

    const auto onPedal = [] (juce::Rectangle<int> r) { return r.translated (pedalArea.getX(), pedalArea.getY()); };

    display.setBounds (onPedal (displayArea));
    rigUp.setBounds (onPedal ({ displayArea.getRight() + 14, displayArea.getY() + 14, 26, 26 }));
    rigDown.setBounds (onPedal ({ displayArea.getRight() + 14, displayArea.getBottom() - 40, 26, 26 }));
    pedalSave.setBounds (onPedal ({ displayArea.getX() + 60, displayArea.getBottom() + 8, 90, 24 }));
    pedalSaveAs.setBounds (onPedal ({ displayArea.getRight() - 150, displayArea.getBottom() + 8, 110, 24 }));

    // The cluster: a label printed above each small button.
    const auto placeInCluster = [&] (juce::Label& label, juce::Button& button, int column, int row)
    {
        const auto x = clusterX + column * clusterColumn, y = clusterTopY + row * clusterRowGap;
        label.setBounds (onPedal ({ x - 8, y, clusterButtonWidth + 16, 14 }));
        button.setBounds (onPedal ({ x, y + 16, clusterButtonWidth, 20 }));
    };
    placeInCluster (undoLabel, undoPedal, 0, 0);
    placeInCluster (redoLabel, redoPedal, 1, 0);
    placeInCluster (bypassLabel, bypassButton, 2, 0);
    placeInCluster (tunerLabel, tunerPedal, 0, 1);
    placeInCluster (muteLabel, mutePedal, 1, 1);
    placeInCluster (tapLabel, tapPedal, 2, 1);
    holdLabel.setBounds (onPedal ({ clusterX + 2 * clusterColumn - 24, clusterTopY + clusterRowGap + 38, clusterButtonWidth + 48, 12 }));

    rigLevelKnob.setBounds (onPedal ({ 940, 30, 96, 96 }));
    rigLevelLabel.setBounds (onPedal ({ 928, 128, 120, 18 }));
    presenceKnob.setBounds (onPedal ({ 1046, 30, 96, 96 }));
    presenceLabel.setBounds (onPedal ({ 1034, 128, 120, 18 }));
    pedalAssign.setBounds (onPedal (assignArea));

    ampSlot.setBounds (onPedal (ampWindow));
    cabSlot.setBounds (onPedal (cabWindow));
    rigSlot.setBounds (onPedal (rigWindow));
    normaliseButton.setBounds (onPedal ({ ampWindow.getX() + 10, ampWindow.getY() + 8, 120, 22 }));

    auto switches = onPedal (footswitchRow);
    const auto switchWidth = switches.getWidth() / 4;
    for (auto& footswitch : footswitches)
        footswitch->setBounds (switches.removeFromLeft (switchWidth));

    pedal.setInsets ({ ampWindow, cabWindow, rigWindow, assignArea }, { topDividerY, bottomDividerY }, badgeArea);

    // Dock: the selected block's knobs, the expression pedal's settings, and the current rig
    dockArea = dockBounds;
    auto dock = dockArea;
    currentRigPanel.setBounds (dock.removeFromRight (currentRigWidth));
    dock.removeFromRight (margin);
    expressionPanel.setBounds (dock.removeFromRight (expressionPanelWidth));
    dock.removeFromRight (margin);
    for (auto& panel : panels)
        panel->setBounds (dock);

    signalChain.setBounds (chainBounds);

    // PRESETS page: covers the pedal and the dock
    rigBrowser.setBounds (pedalArea.getUnion (dockBounds).withX (pedalArea.getX()).withRight (pedalArea.getRight()));
}
