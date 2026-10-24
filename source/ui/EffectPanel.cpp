#include "EffectPanel.h"

#include "ParagonLookAndFeel.h"

namespace
{
    constexpr int maxTileWidth = 104;
    constexpr int tileGap = 8;
    constexpr int groupGap = 22;
}

// One control: a label plus whichever widget suits the parameter's type.
struct EffectPanel::Control
{
    Control (juce::AudioProcessorValueTreeState& state, const juce::String& id, const juce::String& labelText)
    {
        auto* parameter = state.getParameter (id);
        jassert (parameter != nullptr);   // a typo in a panel's parameter list
        label.setText (labelText, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::FontOptions (10.5f, juce::Font::bold));
        label.setColour (juce::Label::textColourId, ParagonColours::textDim);
        label.setMinimumHorizontalScale (0.6f);   // squeeze a long name (PING-PONG) into a narrow card rather than cut it off
        label.setBorderSize ({});

        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (parameter))
        {
            auto box = std::make_unique<juce::ComboBox>();
            box->addItemList (choice->choices, 1);
            for (const auto& item : choice->choices)
                longestChoice = juce::jmax (longestChoice, item.length());
            box->setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff151217));
            box->setColour (juce::ComboBox::outlineColourId, ParagonColours::panelEdge);
            box->setColour (juce::ComboBox::textColourId, ParagonColours::accent);
            box->setColour (juce::ComboBox::arrowColourId, ParagonColours::textDim);
            comboAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, id, *box);
            widget = std::move (box);
        }
        else if (dynamic_cast<juce::AudioParameterBool*> (parameter) != nullptr)
        {
            auto button = std::make_unique<juce::ToggleButton>();
            buttonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, id, *button);
            widget = std::move (button);
        }
        else
        {
            auto slider = std::make_unique<juce::Slider> (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
            slider->setColour (juce::Slider::textBoxTextColourId, ParagonColours::accent);
            slider->setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
            slider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, maxTileWidth, 22);
            sliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, id, *slider);
            // Double-click a knob to reset it.
            slider->setDoubleClickReturnValue (true, parameter->convertFrom0to1 (parameter->getDefaultValue()));
            isKnob = true;
            // The widest value it shows (its ends, e.g. "-24.0 dB"), for laying cards out side by side.
            for (const auto end : { 0.0f, 1.0f })
                valueWidth = juce::jmax (valueWidth, juce::GlyphArrangement::getStringWidthInt (juce::FontOptions (15.0f), parameter->getText (end, 32)) + 6);
            widget = std::move (slider);
        }
    }

    // `wide`: a short, wide card (a panel on two rows), where a knob's value
    // goes beside it rather than below, so the knob can be a good size.
    void setBounds (juce::Rectangle<int> area, bool wide = false)
    {
        // Label at the top of the card, the control filling the rest.
        label.setBounds (area.removeFromTop (22));
        area.removeFromTop (wide ? 0 : 4);
        if (isKnob)
        {
            if (auto* slider = dynamic_cast<juce::Slider*> (widget.get()))
                slider->setTextBoxStyle (wide ? juce::Slider::TextBoxRight : juce::Slider::TextBoxBelow, false,
                                         wide ? juce::jmin (valueWidth, area.getWidth() / 2) : maxTileWidth, 22);
            widget->setBounds (area.reduced (2, 0));
        }
        else if (buttonAttachment != nullptr)
            widget->setBounds (area.withSizeKeepingCentre (26, 26));   // LED in the middle of its card
        else
            widget->setBounds (area.withSizeKeepingCentre (area.getWidth() - 4, 28));
    }

    // Returns true if the control appeared or disappeared, so the panel lays out again.
    bool updateActive()
    {
        const auto active = ! isActive || isActive();
        if (hideWhenInactive)
        {
            const auto changed = shown != active;
            shown = active;
            label.setVisible (active);
            widget->setVisible (active);
            return changed;
        }
        widget->setEnabled (active);
        widget->setAlpha (active ? 1.0f : 0.35f);
        label.setAlpha (active ? 1.0f : 0.35f);
        return false;
    }

    std::function<bool()> isActive;
    bool hideWhenInactive = false;
    bool shown = true;

    juce::Label label;
    std::unique_ptr<juce::Component> widget;
    bool isKnob = false;
    int longestChoice = 0;   // a list's longest entry, in characters
    int valueWidth = 0;      // a knob's widest value, in pixels
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachment;
};

EffectPanel::EffectPanel (juce::AudioProcessorValueTreeState& parameters, std::vector<Group> groupsToShow)
{
    for (auto& group : groupsToShow)
    {
        GroupLayout layout { group.title, {}, {} };
        for (const auto& item : group.items)
        {
            controls.push_back (std::make_unique<Control> (parameters, item.parameterId, item.label));
            auto* control = controls.back().get();
            control->isActive = item.isActive;
            control->hideWhenInactive = item.hideWhenInactive;
            addAndMakeVisible (control->label);
            addAndMakeVisible (*control->widget);
            control->updateActive();
            layout.controls.push_back (control);
        }
        groups.push_back (std::move (layout));
    }

    startTimerHz (10);
}

EffectPanel::~EffectPanel() = default;

void EffectPanel::timerCallback()
{
    auto relayout = false;
    for (auto& control : controls)
        if (control->isActive)
            relayout = control->updateActive() || relayout;
    if (relayout)
    {
        resized();
        repaint();
    }

    if (meterSource != nullptr)
    {
        // Falls back gently, as a meter's needle would, so short dips stay readable.
        const auto target = juce::jmin (0.0f, meterSource());
        const auto next = target < meterDb ? target : meterDb + (target - meterDb) * 0.25f;
        if (std::abs (next - meterDb) > 0.05f)
            repaint (meterArea);
        meterDb = next;
    }
}

void EffectPanel::setMeter (std::function<float()> gainReductionDb)
{
    meterSource = std::move (gainReductionDb);
    startTimerHz (30);   // a meter needs to move smoothly
}

void EffectPanel::setHeader (Icons::Type icon, juce::Colour colour, const juce::String& title)
{
    headerIcon = icon;
    headerColour = colour;
    headerTitle = title;
    repaint();
}

void EffectPanel::paint (juce::Graphics& g)
{
    // Same dark panel as the rest of the dock.
    const auto bounds = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0e0b0f), 0.0f, bounds.getY(),
                                             juce::Colour (0xff09070a), 0.0f, bounds.getBottom(), false));
    g.fillRoundedRectangle (bounds, 8.0f);
    g.setColour (ParagonColours::panelEdge);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 8.0f, 1.0f);

    // Header: the block's icon in its colour, and its name.
    auto header = getLocalBounds().reduced (16, 10).removeFromTop (24).toFloat();
    if (headerTitle.isNotEmpty())
    {
        Icons::draw (g, headerIcon, header.removeFromLeft (26.0f).withSizeKeepingCentre (22.0f, 20.0f), headerColour, 1.6f, true);
        header.removeFromLeft (8.0f);
        g.setColour (ParagonColours::text);
        g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
        g.drawText (headerTitle, header, juce::Justification::centredLeft, false);
    }

    // The gain-reduction meter: a row of LEDs, like a compressor pedal's,
    // lighting from the right as it turns the guitar down further.
    if (meterSource != nullptr && ! meterArea.isEmpty())
    {
        auto area = meterArea.toFloat();
        g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
        g.setColour (ParagonColours::textDim);
        g.drawText ("GAIN REDUCTION", area.removeFromLeft (92.0f), juce::Justification::centredLeft, false);
        auto readout = area.removeFromRight (46.0f);
        g.setColour (meterDb < -0.1f ? headerColour : ParagonColours::textDim);
        g.drawText (juce::String (meterDb, 1) + " dB", readout, juce::Justification::centredRight, false);

        constexpr float steps[] = { -20.0f, -15.0f, -12.0f, -10.0f, -8.0f, -7.0f, -6.0f, -5.0f, -4.0f, -3.0f, -2.0f, -1.0f };
        const auto ledWidth = area.getWidth() / (float) std::size (steps);
        for (size_t i = 0; i < std::size (steps); ++i)
        {
            const auto led = juce::Rectangle<float> (area.getX() + (float) i * ledWidth, area.getY(), ledWidth, area.getHeight())
                                 .withSizeKeepingCentre (7.0f, 7.0f);
            const auto lit = meterDb <= steps[i];
            if (lit)
            {
                g.setColour (headerColour.withAlpha (0.35f));
                g.fillEllipse (led.expanded (2.5f));
            }
            g.setColour (lit ? headerColour : juce::Colour (0xff2a2430));
            g.fillEllipse (led);
        }
    }

    // Group names, when a block has more than one part.
    if (groups.size() > 1)
    {
        g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
        for (const auto& group : groups)
        {
            g.setColour (ParagonColours::textDim);
            g.drawText (group.title, group.area.withHeight (16), juce::Justification::centredLeft, false);
        }
    }

    // A card behind each control, with a thin line of the block's colour along its top.
    for (const auto& tile : tiles)
    {
        const auto card = tile.toFloat();
        g.setColour (juce::Colour (0xff0f0c10));
        g.fillRoundedRectangle (card, 6.0f);
        g.setColour (ParagonColours::panelEdge);
        g.drawRoundedRectangle (card.reduced (0.5f), 6.0f, 1.0f);
        g.setColour (headerColour.withAlpha (0.5f));
        g.fillRoundedRectangle (card.reduced (12.0f, 0.0f).withHeight (2.0f), 1.0f);
    }
}

void EffectPanel::resized()
{
    auto area = getLocalBounds().reduced (16, 10);

    // The meter sits in the header, just after the block's name.
    meterArea = {};
    if (meterSource != nullptr)
    {
        const auto titleWidth = juce::GlyphArrangement::getStringWidthInt (juce::FontOptions (14.0f, juce::Font::bold), headerTitle);
        meterArea = area.withHeight (24).withTrimmedLeft (26 + 8 + titleWidth + 28).withWidth (92 + 12 * 14 + 46);
    }

    // Only the controls showing: a hidden one's neighbours close up.
    std::vector<std::vector<Control*>> shown;
    for (const auto& group : groups)
    {
        shown.emplace_back();
        for (auto* control : group.controls)
            if (control->shown)
                shown.back().push_back (control);
    }

    area.removeFromTop (34);   // header
    if (groups.size() > 1)
        area.removeFromTop (20);   // group names
    area.removeFromBottom (4);

    // Cards as wide as fit (up to a limit), centred as a row. An ON light
    // needs little room and a drop-down list a lot, so cards are sized by
    // what they hold rather than all alike.
    int numControls = 0;
    for (const auto& groupControls : shown)
        numControls += (int) groupControls.size();
    // On a crowded panel (the delay's nine controls) the lists give up a
    // little width so the knobs stay a good size.
    const auto listWeight = numControls >= 9 ? 1.75f : 1.9f;
    // A list whose entries run long (the Whammy-Wah's "22 Down 4th / Down 3rd")
    // gets a wider card, in step with its longest entry, so they read in full.
    const auto weight = [listWeight] (const Control& control)
    {
        if (control.isKnob)
            return 1.0f;
        if (control.buttonAttachment != nullptr)
            return 0.75f;
        if (control.longestChoice <= 6)
            return 1.5f;   // a short list (GLITCH's Post / Pre) needs less room than a long one
        return juce::jmax (listWeight, listWeight * (float) control.longestChoice / 16.0f);
    };
    // A crowded panel with one part (the OBSIDIAN's twelve controls) wraps
    // onto two rows, each laid out like a row of its own, so the knobs
    // stay a good size and the lists' entries read in full.
    if (groups.size() == 1 && numControls > 10)
    {
        // On a wide card a knob's value sits beside it: a long one ("-24.0 dB") needs a wider card.
        const auto wideWeight = [&weight] (const Control& control)
        {
            return control.isKnob ? juce::jmax (1.0f, (float) control.valueWidth / 40.0f) : weight (control);
        };
        tiles.clear();
        groups[0].area = area;
        const auto rowHeight = (area.getHeight() - tileGap) / 2;
        const auto split = shown[0].begin() + (numControls + 1) / 2;
        for (const auto& [first, last] : { std::pair { shown[0].begin(), split }, std::pair { split, shown[0].end() } })
        {
            auto rowArea = area.removeFromTop (rowHeight);
            area.removeFromTop (tileGap);
            float rowWeight = 0.0f;
            for (auto it = first; it != last; ++it)
                rowWeight += wideWeight (**it);
            const auto rowGaps = tileGap * (int) (std::distance (first, last) - 1);
            const auto rowUnit = juce::jmin ((float) maxTileWidth, (float) (rowArea.getWidth() - rowGaps) / juce::jmax (1.0f, rowWeight));
            auto rowWidth = rowGaps;
            for (auto it = first; it != last; ++it)
                rowWidth += juce::roundToInt (rowUnit * wideWeight (**it));
            rowArea = rowArea.withSizeKeepingCentre (rowWidth, rowArea.getHeight());
            for (auto it = first; it != last; ++it)
            {
                const auto tile = rowArea.removeFromLeft (juce::roundToInt (rowUnit * wideWeight (**it)));
                rowArea.removeFromLeft (tileGap);
                tiles.push_back (tile);
                (*it)->setBounds (tile.reduced (4, 6), true);
            }
        }
        return;
    }

    float totalWeight = 0.0f;
    for (const auto& groupControls : shown)
        for (const auto* control : groupControls)
            totalWeight += weight (*control);
    const auto gaps = tileGap * (numControls - (int) groups.size()) + groupGap * ((int) groups.size() - 1);
    const auto unitWidth = juce::jmin ((float) maxTileWidth, (float) (area.getWidth() - gaps) / juce::jmax (1.0f, totalWeight));
    const auto tileWidth = [&] (const Control& control) { return juce::roundToInt (unitWidth * weight (control)); };

    auto totalWidth = gaps;
    for (const auto& groupControls : shown)
        for (const auto* control : groupControls)
            totalWidth += tileWidth (*control);
    area = area.withSizeKeepingCentre (totalWidth, area.getHeight());

    tiles.clear();
    for (size_t g = 0; g < groups.size(); ++g)
    {
        auto& group = groups[g];
        auto groupWidth = tileGap * ((int) shown[g].size() - 1);
        for (const auto* control : shown[g])
            groupWidth += tileWidth (*control);
        group.area = area.removeFromLeft (groupWidth).withTrimmedTop (-20);
        area.removeFromLeft (groupGap);

        auto row = group.area.withTrimmedTop (20);
        for (auto* control : shown[g])
        {
            const auto tile = row.removeFromLeft (tileWidth (*control));
            row.removeFromLeft (tileGap);
            tiles.push_back (tile);
            control->setBounds (tile.reduced (4, 8));
        }
    }
}
