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

        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (parameter))
        {
            auto box = std::make_unique<juce::ComboBox>();
            box->addItemList (choice->choices, 1);
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
            widget = std::move (slider);
        }
    }

    void setBounds (juce::Rectangle<int> area)
    {
        // Label at the top of the card, the control filling the rest.
        label.setBounds (area.removeFromTop (22));
        area.removeFromTop (4);
        if (isKnob)
            widget->setBounds (area.reduced (2, 0));
        else if (buttonAttachment != nullptr)
            widget->setBounds (area.withSizeKeepingCentre (26, 26));   // LED in the middle of its card
        else
            widget->setBounds (area.withSizeKeepingCentre (area.getWidth() - 10, 28));
    }

    void updateActive()
    {
        const auto active = ! isActive || isActive();
        widget->setEnabled (active);
        widget->setAlpha (active ? 1.0f : 0.35f);
        label.setAlpha (active ? 1.0f : 0.35f);
    }

    std::function<bool()> isActive;

    juce::Label label;
    std::unique_ptr<juce::Component> widget;
    bool isKnob = false;
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
            control->updateActive();
            addAndMakeVisible (control->label);
            addAndMakeVisible (*control->widget);
            layout.controls.push_back (control);
        }
        groups.push_back (std::move (layout));
    }

    startTimerHz (10);
}

EffectPanel::~EffectPanel() = default;

void EffectPanel::timerCallback()
{
    for (auto& control : controls)
        if (control->isActive)
            control->updateActive();
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
    area.removeFromTop (34);   // header
    if (groups.size() > 1)
        area.removeFromTop (20);   // group names
    area.removeFromBottom (4);

    // Cards as wide as fit (up to a limit), centred as a row.
    int numControls = 0;
    for (const auto& group : groups)
        numControls += (int) group.controls.size();
    const auto gaps = tileGap * (numControls - (int) groups.size()) + groupGap * ((int) groups.size() - 1);
    const auto tileWidth = juce::jmin (maxTileWidth, (area.getWidth() - gaps) / juce::jmax (1, numControls));
    const auto totalWidth = tileWidth * numControls + gaps;
    area = area.withSizeKeepingCentre (totalWidth, area.getHeight());

    tiles.clear();
    for (auto& group : groups)
    {
        const auto groupWidth = tileWidth * (int) group.controls.size() + tileGap * ((int) group.controls.size() - 1);
        group.area = area.removeFromLeft (groupWidth).withTrimmedTop (-20);
        area.removeFromLeft (groupGap);

        auto row = group.area.withTrimmedTop (20);
        for (auto* control : group.controls)
        {
            const auto tile = row.removeFromLeft (tileWidth);
            row.removeFromLeft (tileGap);
            tiles.push_back (tile);
            control->setBounds (tile.reduced (4, 8));
        }
    }
}
