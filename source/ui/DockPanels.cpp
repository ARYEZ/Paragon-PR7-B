#include "DockPanels.h"

#include "Parameters.h"
#include "ui/ParagonLookAndFeel.h"

namespace DockStyle
{
    void drawPanel (juce::Graphics& g, juce::Rectangle<float> area)
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0e0b0f), 0.0f, area.getY(),
                                                 juce::Colour (0xff09070a), 0.0f, area.getBottom(), false));
        g.fillRoundedRectangle (area, 8.0f);
        g.setColour (ParagonColours::panelEdge);
        g.drawRoundedRectangle (area.reduced (0.5f), 8.0f, 1.0f);
    }

    void drawTitle (juce::Graphics& g, juce::Rectangle<int> area, const juce::String& title)
    {
        g.setColour (ParagonColours::text.withAlpha (0.9f));
        g.setFont (juce::FontOptions (12.5f, juce::Font::bold));
        g.drawText (title, area, juce::Justification::centredLeft, false);
    }
}

namespace
{
    void styleButton (juce::TextButton& button, bool primary)
    {
        button.setColour (juce::TextButton::buttonColourId, primary ? ParagonColours::accentDim.withAlpha (0.6f) : juce::Colour (0xff151217));
        button.setColour (juce::TextButton::textColourOffId, primary ? ParagonColours::accent : ParagonColours::text);
    }

    void styleCombo (juce::ComboBox& box)
    {
        box.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff09070a));
        box.setColour (juce::ComboBox::outlineColourId, ParagonColours::panelEdge);
        box.setColour (juce::ComboBox::textColourId, ParagonColours::text);
        box.setColour (juce::ComboBox::arrowColourId, ParagonColours::text);
    }
}

//==============================================================================
RigNavigator::RigNavigator()
{
    rows = {
        { "All Rigs", {}, Icons::Type::flame, 0, -1, {} },
        { "Favourites", {}, Icons::Type::star, 1, -1, {} },
        { "My Rigs", {}, Icons::Type::user, 2, -1, {} },
        { "Factory Rigs", {}, Icons::Type::factory, 3, -1, {} },
    };
    for (int bank = 0; bank < Rig::numBanks; ++bank)
        rows.push_back ({ "Bank " + Rig::bankName (bank), Rig::bankName (bank), Icons::Type::rig, -1, bank, {} });

    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void RigNavigator::layoutRows()
{
    auto area = getLocalBounds().reduced (12, 10);
    area.removeFromTop (22);
    for (size_t i = 0; i < rows.size(); ++i)
    {
        if (i == 4)
        {
            area.removeFromTop (6);
            banksTitle = area.removeFromTop (20);
        }
        rows[i].area = area.removeFromTop (23);
    }
}

int RigNavigator::rowAt (juce::Point<int> position) const
{
    for (size_t i = 0; i < rows.size(); ++i)
        if (rows[i].area.contains (position))
            return (int) i;
    return -1;
}

void RigNavigator::paint (juce::Graphics& g)
{
    layoutRows();
    DockStyle::drawPanel (g, getLocalBounds().toFloat());
    DockStyle::drawTitle (g, getLocalBounds().reduced (16, 10).removeFromTop (18), "RIG BROWSER");
    DockStyle::drawTitle (g, banksTitle.withTrimmedLeft (4), "BANKS");

    for (size_t i = 0; i < rows.size(); ++i)
    {
        const auto& row = rows[i];
        const auto area = row.area.toFloat();
        const auto isSelected = (int) i == selected;

        if (isSelected)
        {
            g.setColour (ParagonColours::accentDim.withAlpha (0.45f));
            g.fillRoundedRectangle (area, 4.0f);
            g.setColour (ParagonColours::accent.withAlpha (0.7f));
            g.drawRoundedRectangle (area.reduced (0.5f), 4.0f, 1.0f);
        }
        else if ((int) i == hovered)
        {
            g.setColour (juce::Colours::white.withAlpha (0.05f));
            g.fillRoundedRectangle (area, 4.0f);
        }
        else
        {
            g.setColour (ParagonColours::panelEdge.withAlpha (0.6f));
            g.drawHorizontalLine (row.area.getBottom() - 1, area.getX() + 4.0f, area.getRight() - 4.0f);
        }

        auto content = area.reduced (8.0f, 0.0f);
        const auto colour = isSelected ? ParagonColours::accent : ParagonColours::text.withAlpha (0.8f);
        auto iconArea = content.removeFromLeft (16.0f);
        if (row.letter.isNotEmpty())
        {
            g.setColour (colour);
            g.setFont (juce::FontOptions (12.5f, juce::Font::bold));
            g.drawText (row.letter, iconArea, juce::Justification::centred, false);
        }
        else
        {
            Icons::draw (g, row.icon, iconArea.withSizeKeepingCentre (13.0f, 13.0f), colour, 1.3f);
        }
        content.removeFromLeft (10.0f);
        g.setColour (colour);
        g.setFont (juce::FontOptions (12.5f));
        g.drawText (row.text, content, juce::Justification::centredLeft, false);
    }
}

void RigNavigator::mouseMove (const juce::MouseEvent& event)
{
    if (const auto row = rowAt (event.getPosition()); row != hovered)
    {
        hovered = row;
        repaint();
    }
}

void RigNavigator::mouseExit (const juce::MouseEvent&)
{
    hovered = -1;
    repaint();
}

void RigNavigator::mouseUp (const juce::MouseEvent& event)
{
    const auto row = rowAt (event.getPosition());
    if (row < 0 || ! event.mouseWasClicked())
        return;

    selected = row;
    repaint();
    if (onShow)
        onShow (juce::jmax (0, rows[(size_t) row].filter), rows[(size_t) row].bank);
}

//==============================================================================
SelectedRigPanel::SelectedRigPanel (ParagonX1AudioProcessor& p) : processor (p)
{
    for (int bank = 0; bank < Rig::numBanks; ++bank)
        bankBox.addItem (Rig::bankName (bank) + "   Bank " + Rig::bankName (bank), bank + 1);
    for (int number = 1; number <= Rig::maxNumber; ++number)
        numberBox.addItem (juce::String (number).paddedLeft ('0', 3), number);

    for (auto* box : { &bankBox, &numberBox })
    {
        styleCombo (*box);
        box->onChange = [this] { refile(); };
        box->setTooltip ("Where this rig is filed. Change it to move one of your own rigs; factory rigs stay where they are.");
        addAndMakeVisible (*box);
    }

    styleButton (save, true);
    styleButton (saveAs, false);
    save.onClick = [this] { if (onSave) onSave(); };
    saveAs.onClick = [this] { if (onSaveAs) onSaveAs(); };
    addAndMakeVisible (save);
    addAndMakeVisible (saveAs);

    refresh();
}

void SelectedRigPanel::refresh()
{
    const juce::ScopedValueSetter<bool> guard (refreshing, true);
    const auto current = processor.getCurrentRig();
    const auto* entry = processor.findCurrentRigEntry();

    name = current.name + (processor.isRigModified() ? " *" : "");
    hasEntry = entry != nullptr;
    favourite = entry != nullptr && entry->rig.favourite;
    canRefile = entry != nullptr && ! entry->isFactory;

    bankBox.setSelectedId (entry != nullptr ? entry->rig.bank + 1 : 0, juce::dontSendNotification);
    numberBox.setSelectedId (entry != nullptr ? entry->rig.number : 0, juce::dontSendNotification);
    bankBox.setTextWhenNothingSelected ("Not saved");
    numberBox.setTextWhenNothingSelected ("---");
    bankBox.setEnabled (canRefile);
    numberBox.setEnabled (canRefile);
    repaint();
}

void SelectedRigPanel::refile()
{
    if (refreshing || ! canRefile)
        return;

    if (const auto* entry = processor.findCurrentRigEntry())
    {
        auto details = entry->rig;
        details.bank = bankBox.getSelectedId() - 1;
        details.number = numberBox.getSelectedId();
        processor.updateRigDetails (*entry, details);
    }
    refresh();
}

void SelectedRigPanel::paint (juce::Graphics& g)
{
    DockStyle::drawPanel (g, getLocalBounds().toFloat());
    DockStyle::drawTitle (g, getLocalBounds().reduced (16, 10).removeFromTop (18), "SELECTED RIG");

    // Name box with the favourite star, as in the mockup.
    g.setColour (juce::Colour (0xff09070a));
    g.fillRoundedRectangle (nameBox.toFloat(), 6.0f);
    g.setColour (ParagonColours::panelEdge);
    g.drawRoundedRectangle (nameBox.toFloat().reduced (0.5f), 6.0f, 1.0f);
    g.setColour (ParagonColours::text);
    g.setFont (juce::FontOptions (21.0f));
    g.drawFittedText (name, nameBox.reduced (14, 0).withTrimmedRight (starArea.getWidth()), juce::Justification::centredLeft, 1, 0.8f);

    if (hasEntry)
    {
        auto star = Icons::get (Icons::Type::star);
        const auto area = starArea.toFloat().withSizeKeepingCentre (22.0f, 22.0f);
        star.applyTransform (juce::AffineTransform::scale (area.getWidth()).translated (area.getX(), area.getY()));
        g.setColour (ParagonColours::accent);
        if (favourite)
            g.fillPath (star);
        else
            g.strokePath (star, juce::PathStrokeType (1.4f));
    }

    g.setColour (ParagonColours::textDim);
    g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    g.drawText ("BANK", bankLabel, juce::Justification::centredLeft, false);
    g.drawText ("NUMBER", numberLabel, juce::Justification::centredLeft, false);
}

void SelectedRigPanel::resized()
{
    auto area = getLocalBounds().reduced (16, 10);
    area.removeFromTop (24);
    nameBox = area.removeFromTop (42);
    starArea = nameBox.withLeft (nameBox.getRight() - 44);
    area.removeFromTop (10);

    auto labels = area.removeFromTop (18);
    const auto half = (area.getWidth() - 16) / 2;
    bankLabel = labels.removeFromLeft (half);
    numberLabel = labels.withTrimmedLeft (16);

    auto combos = area.removeFromTop (30);
    bankBox.setBounds (combos.removeFromLeft (half));
    numberBox.setBounds (combos.withTrimmedLeft (16));

    area.removeFromTop (18);
    auto buttons = area.removeFromTop (32);
    save.setBounds (buttons.removeFromLeft (half));
    saveAs.setBounds (buttons.withTrimmedLeft (16));
}

void SelectedRigPanel::mouseUp (const juce::MouseEvent& event)
{
    if (! event.mouseWasClicked() || ! starArea.contains (event.getPosition()))
        return;

    if (const auto* entry = processor.findCurrentRigEntry())
    {
        processor.getRigLibrary().setFavourite (*entry, ! entry->rig.favourite);
        refresh();
    }
}

//==============================================================================
RigSummary::RigSummary (ParagonX1AudioProcessor& p) : processor (p)
{
    startTimerHz (5);   // the cards follow knob moves, automation and rig changes
    refresh();
}

juce::String RigSummary::text (const char* parameterId) const
{
    return processor.getParameters().getParameter (parameterId)->getCurrentValueAsText();
}

float RigSummary::value (const char* parameterId) const
{
    return processor.getParameters().getRawParameterValue (parameterId)->load();
}

juce::String RigSummary::eqSummary() const
{
    // The biggest change, e.g. "+4.0 dB @ 1K", or "Flat".
    int biggest = -1;
    float biggestDb = 0.0f;
    for (int band = 0; band < ParamIDs::eqBands.size(); ++band)
    {
        const auto db = processor.getParameters().getRawParameterValue (ParamIDs::eqBands[band])->load();
        if (std::abs (db) > std::abs (biggestDb) + 0.05f)
        {
            biggest = band;
            biggestDb = db;
        }
    }
    if (biggest < 0)
        return "Flat";
    return (biggestDb > 0.0f ? "+" : "") + juce::String (biggestDb, 1) + " dB @ " + ParamIDs::eqBandNames[biggest];
}

void RigSummary::refresh()
{
    const auto on = [this] (const char* id) { return value (id) > 0.5f; };
    const auto fileName = [] (const SlotStatus& status, const juce::String& none)
    {
        return status.file != juce::File() ? status.file.getFileNameWithoutExtension() : none;
    };

    const auto amp = processor.getAmpStatus(), cab = processor.getCabStatus();
    const auto gateOn = value (ParamIDs::gateThreshold) > effects::CompGate::gateOffDb;
    const auto synced = value (ParamIDs::delaySync) > 0.5f;

    std::vector<Card> newCards {
        { "COMP/GATE", gateOn ? "Comp + Gate" : "Studio Comp", text (ParamIDs::compThreshold) + "  " + text (ParamIDs::compRatio),
          Icons::Type::comp, ParagonColours::comp, on (ParamIDs::compOn), {} },
        { "FLT/PITCH", text (ParamIDs::fpType),
          juce::roundToInt (value (ParamIDs::fpType)) == (int) effects::FilterPitch::Type::pitchShifter ? text (ParamIDs::fpInterval)
                                                                                                         : "Sens " + text (ParamIDs::fpSensitivity),
          Icons::Type::filterPitch, ParagonColours::filterPitch, on (ParamIDs::fpOn), {} },
        { "OD/DIST", juce::roundToInt (value (ParamIDs::driveModel)) == (int) effects::Drive::Model::screamer ? "Screamer" : "Classic OD",
          "Drive " + text (ParamIDs::driveGain), Icons::Type::drive, ParagonColours::drive, on (ParamIDs::driveOn), {} },
        { "AMP", fileName (amp, "No model"), on (ParamIDs::ampNormalise) ? "Normalised" : "Raw level",
          Icons::Type::amp, ParagonColours::amp, on (ParamIDs::ampOn) && amp.file != juce::File(), {} },
        { "CAB IR", fileName (cab, "No IR"), amp.file != juce::File() || cab.file != juce::File() ? cab.detail : juce::String(),
          Icons::Type::cabinet, ParagonColours::cab, on (ParamIDs::cabOn) && cab.file != juce::File(), {} },
        { "EQ", "9-Band EQ", eqSummary(), Icons::Type::eq, ParagonColours::eq, on (ParamIDs::eqOn), {} },
        { "MOD", text (ParamIDs::modType), "Depth " + text (ParamIDs::modDepth), Icons::Type::modulation, ParagonColours::modulation,
          on (ParamIDs::modOn), {} },
        { "DELAY", synced ? "Synced Delay" : "Digital Delay",
          (synced ? text (ParamIDs::delaySync) : text (ParamIDs::delayTime)) + "  " + text (ParamIDs::delayMix),
          Icons::Type::delay, ParagonColours::delay, on (ParamIDs::delayOn), {} },
        { "REVERB", "Hall Reverb", "Mix " + text (ParamIDs::reverbMix), Icons::Type::reverb, ParagonColours::reverb, on (ParamIDs::reverbOn), {} },
    };

    juce::StringArray newTags;
    const auto* entry = processor.findCurrentRigEntry();
    if (entry != nullptr)
        newTags = entry->rig.tags;
    const auto newCanEdit = entry != nullptr && ! entry->isFactory;

    // Repaint only when something visible changed.
    auto changed = newTags != tags || newCanEdit != canEditTags || newCards.size() != cards.size();
    for (size_t i = 0; ! changed && i < cards.size(); ++i)
        changed = newCards[i].line1 != cards[i].line1 || newCards[i].line2 != cards[i].line2 || newCards[i].on != cards[i].on;

    if (! changed)
        return;

    for (size_t i = 0; i < newCards.size() && i < cards.size(); ++i)
        newCards[i].area = cards[i].area;
    cards = std::move (newCards);
    tags = newTags;
    canEditTags = newCanEdit;
    resized();
    repaint();
}

void RigSummary::resized()
{
    auto area = getLocalBounds().reduced (16, 10);
    tagsTitle = area.removeFromTop (18);
    area.removeFromTop (6);

    // Tag chips, sized to their text, then the + chip.
    auto chips = area.removeFromTop (24).toFloat();
    tagAreas.clear();
    const auto font = juce::FontOptions (12.0f);
    for (const auto& tag : tags)
    {
        const auto width = (float) juce::GlyphArrangement::getStringWidthInt (font, tag) + 26.0f;
        if (width > chips.getWidth() - 34.0f)
            break;
        tagAreas.push_back (chips.removeFromLeft (width));
        chips.removeFromLeft (8.0f);
    }
    plusArea = chips.removeFromLeft (24.0f);

    area.removeFromTop (14);
    summaryTitle = area.removeFromTop (18);
    area.removeFromTop (6);

    // Seven cards with a small arrow between each, as in the mockup.
    const auto numCards = (float) cards.size();
    const auto arrowGap = 12.0f;
    const auto cardWidth = ((float) area.getWidth() - arrowGap * (numCards - 1.0f)) / numCards;
    auto row = area.toFloat();
    for (auto& card : cards)
    {
        card.area = row.removeFromLeft (cardWidth);
        row.removeFromLeft (arrowGap);
    }
}

void RigSummary::paint (juce::Graphics& g)
{
    DockStyle::drawPanel (g, getLocalBounds().toFloat());
    DockStyle::drawTitle (g, tagsTitle, "TAGS");
    DockStyle::drawTitle (g, summaryTitle, "RIG SUMMARY");

    g.setFont (juce::FontOptions (12.0f));
    for (size_t i = 0; i < tagAreas.size(); ++i)
    {
        g.setColour (juce::Colour (0xff151217));
        g.fillRoundedRectangle (tagAreas[i], 4.0f);
        g.setColour (ParagonColours::panelEdge.brighter (0.2f));
        g.drawRoundedRectangle (tagAreas[i].reduced (0.5f), 4.0f, 1.0f);
        g.setColour (ParagonColours::text.withAlpha (0.85f));
        g.drawText (tags[(int) i], tagAreas[i], juce::Justification::centred, false);
    }
    if (tags.isEmpty())
    {
        g.setColour (ParagonColours::textDim);
        g.drawText (canEditTags ? "No tags yet" : "Save the rig to tag it",
                    plusArea.withX (plusArea.getRight() + 8.0f).withWidth (200.0f), juce::Justification::centredLeft, false);
    }
    Icons::draw (g, Icons::Type::plus, plusArea.withSizeKeepingCentre (14.0f, 14.0f),
                 canEditTags ? ParagonColours::text : ParagonColours::textDim.withAlpha (0.4f), 1.5f);

    for (size_t i = 0; i < cards.size(); ++i)
    {
        const auto& card = cards[i];
        const auto area = card.area;
        const auto hover = (int) i == hoveredCard;

        g.setColour (hover ? juce::Colour (0xff19151b) : juce::Colour (0xff0f0c10));
        g.fillRoundedRectangle (area, 5.0f);
        g.setColour (hover ? card.colour.withAlpha (0.7f) : ParagonColours::panelEdge.brighter (card.on ? 0.25f : 0.0f));
        g.drawRoundedRectangle (area.reduced (0.5f), 5.0f, 1.0f);

        auto content = area.reduced (5.0f, 8.0f);
        g.setColour (ParagonColours::text.withAlpha (0.75f));
        g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
        g.drawFittedText (card.title, content.removeFromTop (14.0f).toNearestInt(), juce::Justification::centred, 1, 0.7f);

        const auto iconArea = content.removeFromTop (content.getHeight() * 0.5f).withSizeKeepingCentre (40.0f, 34.0f);
        Icons::draw (g, card.icon, iconArea, card.on ? card.colour : card.colour.withAlpha (0.3f), 1.7f, card.on);

        g.setColour (card.on ? ParagonColours::text.withAlpha (0.9f) : ParagonColours::textDim);
        g.setFont (juce::FontOptions (10.5f));
        g.drawFittedText (card.line1, content.removeFromTop (content.getHeight() * 0.5f).toNearestInt(), juce::Justification::centred, 1, 0.75f);
        g.setColour (ParagonColours::textDim);
        g.drawFittedText (card.on ? card.line2 : juce::String ("OFF"), content.toNearestInt(), juce::Justification::centredTop, 1, 0.75f);

        if (i + 1 < cards.size())
            Icons::draw (g, Icons::Type::chevronRight,
                         juce::Rectangle<float> (8.0f, 10.0f).withCentre ({ area.getRight() + 6.0f, area.getCentreY() }),
                         ParagonColours::textDim, 1.3f);
    }
}

void RigSummary::mouseMove (const juce::MouseEvent& event)
{
    auto card = -1;
    for (size_t i = 0; i < cards.size(); ++i)
        if (cards[i].area.contains (event.position))
            card = (int) i;

    const auto overTag = plusArea.contains (event.position) || std::any_of (tagAreas.begin(), tagAreas.end(),
                                                                             [&] (auto& r) { return r.contains (event.position); });
    setMouseCursor (card >= 0 || (overTag && canEditTags) ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
    // The cards' titles are abbreviated to fit; the tooltip names the block in full.
    static const juce::StringArray blockNames { "the compressor and gate", "the filter / pitch block", "the overdrive",
                                                "the amp", "the cabinet", "the graphic EQ", "the modulation",
                                                "the delay", "the reverb" };
    setTooltip (card >= 0 ? "Edit " + blockNames[card]
                          : overTag && canEditTags ? juce::String ("+ adds a tag; click a tag to remove it") : juce::String());

    if (card != hoveredCard)
    {
        hoveredCard = card;
        repaint();
    }
}

void RigSummary::mouseExit (const juce::MouseEvent&)
{
    hoveredCard = -1;
    repaint();
}

void RigSummary::mouseUp (const juce::MouseEvent& event)
{
    if (! event.mouseWasClicked())
        return;

    for (size_t i = 0; i < cards.size(); ++i)
        if (cards[i].area.contains (event.position) && onSelectBlock)
            return onSelectBlock ((int) i);

    if (! canEditTags)
        return;

    if (plusArea.contains (event.position))
        return addTag();

    for (size_t i = 0; i < tagAreas.size(); ++i)
        if (tagAreas[i].contains (event.position))
        {
            juce::PopupMenu menu;
            menu.addItem ("Remove tag \"" + tags[(int) i] + "\"", [this, tag = tags[(int) i]] { removeTag (tag); });
            menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this));
            return;
        }
}

void RigSummary::setTags (const juce::StringArray& newTags)
{
    if (const auto* entry = processor.findCurrentRigEntry())
    {
        auto details = entry->rig;
        details.tags = newTags;
        processor.updateRigDetails (*entry, details);
    }
    refresh();
}

void RigSummary::removeTag (const juce::String& tag)
{
    auto newTags = tags;
    newTags.removeString (tag);
    setTags (newTags);
}

void RigSummary::addTag()
{
    dialog = std::make_unique<juce::AlertWindow> ("Add a tag", "For example: Rock, Lead, Clean, Worship.", juce::MessageBoxIconType::NoIcon, this);
    dialog->addTextEditor ("tag", {}, "Tag:");
    dialog->addButton ("ADD", 1, juce::KeyPress (juce::KeyPress::returnKey));
    dialog->addButton ("CANCEL", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    dialog->enterModalState (true, juce::ModalCallbackFunction::create ([this] (int result)
    {
        const auto tag = dialog->getTextEditorContents ("tag").trim();
        dialog.reset();
        if (result == 1 && tag.isNotEmpty() && ! tags.contains (tag, true))
        {
            auto newTags = tags;
            newTags.add (tag);
            setTags (newTags);
        }
    }));
}

//==============================================================================
SettingsPage::SettingsPage (ParagonX1AudioProcessor& p)
    : processor (p),
      muteAttachment (p.getParameters(), ParamIDs::tunerMute, muteWhileTuning)
{
    reference.setRange (430.0, 450.0, 1.0);
    reference.setTextValueSuffix (" Hz");
    reference.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    reference.setColour (juce::Slider::textBoxTextColourId, ParagonColours::accent);
    reference.setColour (juce::Slider::trackColourId, ParagonColours::accent);
    reference.setColour (juce::Slider::thumbColourId, ParagonColours::accent);
    reference.setDoubleClickReturnValue (true, 440.0);
    reference.onValueChange = [this] { processor.setTunerReferenceHz (reference.getValue()); };
    reference.setTooltip ("The pitch the tuner calls A4. 440 Hz is standard; some bands tune to 432 or 442. Double-click for 440.");

    muteWhileTuning.setColour (juce::ToggleButton::textColourId, ParagonColours::text);

    for (int cc = 1; cc <= 127; ++cc)
        ccBox.addItem ("CC " + juce::String (cc) + (cc == 11 ? "  (Expression, standard)" : cc == 7 ? "  (Volume)" : cc == 4 ? "  (Foot controller)" : ""), cc);
    styleCombo (ccBox);
    ccBox.onChange = [this] { processor.setExpressionCc (ccBox.getSelectedId()); };

    styleButton (openFolder, false);
    openFolder.onClick = [this]
    {
        processor.getRigLibrary().getFolder().createDirectory();
        processor.getRigLibrary().getFolder().startAsProcess();
    };

    for (auto* component : std::initializer_list<juce::Component*> { &reference, &muteWhileTuning, &ccBox, &openFolder })
        addAndMakeVisible (component);
}

void SettingsPage::visibilityChanged()
{
    // Settings can change from a loaded project, so read them each time the page opens.
    reference.setValue (processor.getTunerReferenceHz(), juce::dontSendNotification);
    ccBox.setSelectedId (processor.getExpressionCc(), juce::dontSendNotification);
}

void SettingsPage::paint (juce::Graphics& g)
{
    const auto section = [&g] (juce::Rectangle<int> area, const juce::String& title, const juce::String& body)
    {
        DockStyle::drawPanel (g, area.toFloat());
        auto inner = area.reduced (24, 18);
        g.setColour (ParagonColours::text);
        g.setFont (juce::FontOptions (16.0f, juce::Font::bold));
        g.drawText (title, inner.removeFromTop (24), juce::Justification::centredLeft, false);
        g.setColour (ParagonColours::textDim);
        g.setFont (juce::FontOptions (13.0f));
        g.drawFittedText (body, inner.removeFromTop (60), juce::Justification::topLeft, 4, 1.0f);
    };

    section (tunerArea, "TUNER", "Hold TAP or press TUNER on the pedal to tune. Reference pitch for A4:");
    section (expressionArea, "EXPRESSION PEDAL",
             "A MIDI expression pedal moves the on-screen pedal. Pick the MIDI controller it sends "
             "(most send CC 11), and route the MIDI track to Paragon X1 in your DAW.");
    section (rigsArea, "RIGS", "Your rigs are files in this folder, so you can back them up or share them:\n"
                               + processor.getRigLibrary().getFolder().getFullPathName());
    section (aboutArea, "ABOUT",
             juce::String ("Paragon X1 v") + JucePlugin_VersionString
                 + ". Free and open source under the GNU AGPLv3.\n"
                   "Built with JUCE (AGPLv3), NeuralAmpModelerCore (MIT), Eigen (MPL 2.0) and the Inter typeface (SIL OFL).");
}

void SettingsPage::resized()
{
    auto area = getLocalBounds();
    auto top = area.removeFromTop (area.getHeight() / 2 - 6);
    area.removeFromTop (12);
    tunerArea = top.removeFromLeft (top.getWidth() / 2 - 6);
    expressionArea = top.withTrimmedLeft (12);
    rigsArea = area.removeFromLeft (area.getWidth() / 2 - 6);
    aboutArea = area.withTrimmedLeft (12);

    auto tunerControls = tunerArea.reduced (24, 18).withTrimmedTop (90);
    reference.setBounds (tunerControls.removeFromTop (32).withWidth (juce::jmin (420, tunerControls.getWidth())));
    tunerControls.removeFromTop (12);
    muteWhileTuning.setBounds (tunerControls.removeFromTop (26).withWidth (360));

    ccBox.setBounds (expressionArea.reduced (24, 18).withTrimmedTop (100).removeFromTop (30).withWidth (300));
    openFolder.setBounds (rigsArea.reduced (24, 18).withTrimmedTop (100).removeFromTop (32).withWidth (200));
}
