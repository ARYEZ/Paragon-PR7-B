#include "EffectPanel.h"

#include "ParagonLookAndFeel.h"

namespace
{
    constexpr int controlWidth = 84;
    constexpr int groupGap = 24;
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
        label.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        label.setColour (juce::Label::textColourId, ParagonColours::textDim);

        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (parameter))
        {
            auto box = std::make_unique<juce::ComboBox>();
            box->addItemList (choice->choices, 1);
            box->setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1c1c20));
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
            slider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, controlWidth, 18);
            sliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, id, *slider);
            // Double-click a knob to reset it.
            slider->setDoubleClickReturnValue (true, parameter->convertFrom0to1 (parameter->getDefaultValue()));
            isKnob = true;
            widget = std::move (slider);
        }
    }

    void setBounds (juce::Rectangle<int> area)
    {
        label.setBounds (area.removeFromTop (16));
        area.removeFromTop (8);
        if (isKnob)
            widget->setBounds (area.withTrimmedTop (-8));
        else if (buttonAttachment != nullptr)
            widget->setBounds (area.removeFromTop (26).withSizeKeepingCentre (26, 26));   // LED, centred under its label
        else
            widget->setBounds (area.removeFromTop (26).reduced (4, 0));
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

void EffectPanel::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setColour (ParagonColours::panel);
    g.fillRoundedRectangle (bounds, 8.0f);
    g.setColour (ParagonColours::panelEdge);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 8.0f, 1.0f);

    g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    for (const auto& group : groups)
    {
        g.setColour (ParagonColours::text);
        g.drawText (group.title, group.area.withHeight (18), juce::Justification::centredLeft, false);
        g.setColour (ParagonColours::panelEdge);
        g.drawHorizontalLine (group.area.getY() + 20, (float) group.area.getX(), (float) group.area.getRight());
    }
}

void EffectPanel::resized()
{
    auto area = getLocalBounds().reduced (14, 10);

    // Centre the groups as a whole across the panel.
    int totalWidth = 0;
    for (const auto& group : groups)
        totalWidth += (int) group.controls.size() * controlWidth;
    totalWidth += groupGap * ((int) groups.size() - 1);
    const auto scale = juce::jmin (1.0f, (float) area.getWidth() / (float) juce::jmax (1, totalWidth));
    const auto width = (int) ((float) controlWidth * scale);
    area = area.withSizeKeepingCentre (juce::jmin (area.getWidth(), totalWidth), area.getHeight());

    for (auto& group : groups)
    {
        group.area = area.removeFromLeft (width * (int) group.controls.size());
        area.removeFromLeft (groupGap);

        auto controlsArea = group.area.withTrimmedTop (26);
        for (auto* control : group.controls)
            control->setBounds (controlsArea.removeFromLeft (width).reduced (2, 0));
    }
}
