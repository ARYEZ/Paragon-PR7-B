#include "ModelSlot.h"

#include "ParagonLookAndFeel.h"

ModelSlot::ModelSlot (const juce::String& titleText, Icons::Type iconToShow, juce::Colour iconColourToUse,
                      const juce::String& wildcardToUse, const juce::String& hintText,
                      juce::AudioProcessorValueTreeState& parameters, const juce::String& powerParameterId)
    : title (titleText),
      wildcard (wildcardToUse),
      hint (hintText),
      icon (iconToShow),
      iconColour (iconColourToUse),
      powerAttachment (parameters, powerParameterId, power),
      previous (SlotWindow::makeArrow (true)),
      next (SlotWindow::makeArrow (false))
{
    power.setTooltip ("Turn the " + title.toLowerCase() + " on or off");
    previous->setTooltip ("Previous file in this folder");
    next->setTooltip ("Next file in this folder");
    clear.setTooltip ("Unload");
    clear.getProperties().set (ParagonLookAndFeel::pedalButtonProperty, true);

    previous->onClick = [this] { step (-1); };
    next->onClick = [this] { step (1); };
    clear.onClick = [this] { if (onClear) onClear(); };

    for (auto* component : std::initializer_list<juce::Component*> { &power, previous.get(), next.get(), &clear })
        addAndMakeVisible (component);

    setStatus ({});
}

void ModelSlot::setStatus (const SlotStatus& newStatus)
{
    status = newStatus;
    const auto hasFile = status.file != juce::File();
    previous->setEnabled (hasFile);
    next->setEnabled (hasFile);
    clear.setVisible (hasFile);
    repaint();
}

bool ModelSlot::matchesWildcard (const juce::File& file) const
{
    for (const auto& pattern : juce::StringArray::fromTokens (wildcard, ";", {}))
        if (file.getFileName().matchesWildcard (pattern, true))
            return true;
    return false;
}

void ModelSlot::chooseFile()
{
    const auto startFolder = status.file != juce::File()
                               ? status.file.getParentDirectory()
                               : juce::File::getSpecialLocation (juce::File::userDocumentsDirectory);

    chooser = std::make_unique<juce::FileChooser> ("Load " + title.toLowerCase(), startFolder, wildcard);
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [this] (const juce::FileChooser& fc)
                          {
                              if (const auto file = fc.getResult(); file.existsAsFile() && onLoad)
                                  onLoad (file);
                          });
}

void ModelSlot::step (int direction)
{
    if (status.file == juce::File() || ! onLoad)
        return;

    juce::Array<juce::File> siblings;
    for (const auto& file : status.file.getParentDirectory().findChildFiles (juce::File::findFiles, false))
        if (matchesWildcard (file))
            siblings.add (file);

    if (siblings.isEmpty())
        return;

    std::sort (siblings.begin(), siblings.end(), [] (const juce::File& a, const juce::File& b)
    {
        return a.getFileName().compareNatural (b.getFileName()) < 0;
    });

    // Wrap around at either end. If the current file is gone, start from the first.
    const auto index = siblings.indexOf (status.file);
    const auto count = siblings.size();
    const auto nextIndex = index < 0 ? 0 : (index + direction + count) % count;
    onLoad (siblings[nextIndex]);
}

void ModelSlot::paint (juce::Graphics& g)
{
    SlotWindow::drawTitle (g, layout, title);

    const auto hasFile = status.file != juce::File();
    SlotWindow::drawButton (g, layout.button, icon, hasFile ? iconColour : iconColour.withAlpha (0.5f),
                            hasFile ? status.file.getFileNameWithoutExtension() : juce::String(),
                            buttonHover, dragHover);

    const auto isError = status.error.isNotEmpty() && ! status.loading;
    const auto caption = status.loading ? juce::String ("Loading...")
                       : isError         ? status.error
                       : hasFile         ? status.detail
                                         : hint;
    SlotWindow::drawCaption (g, layout.caption, caption, isError);
}

void ModelSlot::resized()
{
    layout = SlotWindow::layout (getLocalBounds());

    // Power LED just right of the title text, as in the mockup.
    const auto titleWidth = juce::GlyphArrangement::getStringWidthInt (juce::FontOptions (14.0f, juce::Font::bold), title);
    power.setBounds (juce::Rectangle<int> (20, 20).withCentre ({ layout.title.getCentreX() + titleWidth / 2 + 14, layout.title.getCentreY() }));
    clear.setBounds (layout.corner.withSizeKeepingCentre (20, 18));
    previous->setBounds (layout.previous);
    next->setBounds (layout.next);
}

void ModelSlot::mouseMove (const juce::MouseEvent& event)
{
    const auto hover = layout.button.contains (event.getPosition());
    if (hover != buttonHover)
    {
        buttonHover = hover;
        repaint (layout.button.expanded (4));
    }
}

void ModelSlot::mouseExit (const juce::MouseEvent&)
{
    buttonHover = false;
    repaint (layout.button.expanded (4));
}

void ModelSlot::mouseUp (const juce::MouseEvent& event)
{
    if (! event.mouseWasClicked())
        return;
    if (layout.button.contains (event.getPosition()))
        chooseFile();
    else if (layout.title.contains (event.getPosition()) && onTitleClicked)
        onTitleClicked();
}

bool ModelSlot::isInterestedInFileDrag (const juce::StringArray& files)
{
    return files.size() == 1 && matchesWildcard (juce::File (files[0]));
}

void ModelSlot::fileDragEnter (const juce::StringArray&, int, int)
{
    dragHover = true;
    repaint();
}

void ModelSlot::fileDragExit (const juce::StringArray&)
{
    dragHover = false;
    repaint();
}

void ModelSlot::filesDropped (const juce::StringArray& files, int, int)
{
    dragHover = false;
    repaint();
    if (onLoad && ! files.isEmpty())
        onLoad (juce::File (files[0]));
}
