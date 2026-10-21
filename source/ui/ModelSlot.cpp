#include "ModelSlot.h"

#include "ParagonLookAndFeel.h"

ModelSlot::ModelSlot (const juce::String& titleText, const juce::String& wildcardToUse, const juce::String& emptyTextToUse,
                      juce::AudioProcessorValueTreeState& parameters, const juce::String& powerParameterId)
    : title (titleText),
      wildcard (wildcardToUse),
      emptyText (emptyTextToUse),
      powerAttachment (parameters, powerParameterId, power)
{
    power.setTooltip ("Turn the " + title.toLowerCase() + " on or off");
    previous.setTooltip ("Previous file in this folder");
    next.setTooltip ("Next file in this folder");
    clear.setTooltip ("Unload");

    previous.onClick = [this] { step (-1); };
    next.onClick = [this] { step (1); };
    clear.onClick = [this] { if (onClear) onClear(); };

    for (auto* component : std::initializer_list<juce::Component*> { &power, &previous, &next, &clear })
        addAndMakeVisible (component);

    setStatus ({});
}

void ModelSlot::setStatus (const SlotStatus& newStatus)
{
    status = newStatus;
    const auto hasFile = status.file != juce::File();
    previous.setEnabled (hasFile);
    next.setEnabled (hasFile);
    clear.setEnabled (hasFile);
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
    const auto bounds = getLocalBounds().toFloat();
    g.setColour (ParagonColours::panel);
    g.fillRoundedRectangle (bounds, 8.0f);
    g.setColour (dragHover ? ParagonColours::accent : ParagonColours::panelEdge);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 8.0f, dragHover ? 2.0f : 1.0f);

    g.setColour (ParagonColours::text);
    g.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    g.drawText (title, titleArea, juce::Justification::centred, false);

    // The display, like the black buttons in the mockup's slots.
    const auto display = displayArea.toFloat();
    g.setColour (juce::Colour (0xff060607));
    g.fillRoundedRectangle (display, 6.0f);
    g.setColour (ParagonColours::panelEdge);
    g.drawRoundedRectangle (display.reduced (0.5f), 6.0f, 1.0f);

    auto textArea = displayArea.reduced (10, 8);
    const auto hasFile = status.file != juce::File();

    g.setFont (juce::FontOptions (16.0f, juce::Font::bold));
    g.setColour (hasFile ? ParagonColours::accent : ParagonColours::textDim);
    g.drawFittedText (hasFile ? status.file.getFileNameWithoutExtension() : emptyText,
                      textArea.removeFromTop (textArea.getHeight() / 2), juce::Justification::centredBottom, 2);

    juce::String subtitle = status.loading ? "Loading..." : status.error.isNotEmpty() ? status.error : status.detail;
    g.setFont (juce::FontOptions (12.0f));
    g.setColour (status.error.isNotEmpty() && ! status.loading ? ParagonColours::meterRed : ParagonColours::textDim);
    g.drawFittedText (subtitle, textArea.withTrimmedTop (4), juce::Justification::centredTop, 2);
}

void ModelSlot::resized()
{
    auto area = getLocalBounds().reduced (10);

    titleArea = area.removeFromTop (24);
    power.setBounds (titleArea.removeFromRight (24).reduced (2));
    clear.setBounds (titleArea.removeFromLeft (24).reduced (2));

    area.removeFromTop (6);
    previous.setBounds (area.removeFromLeft (28).withSizeKeepingCentre (28, 40));
    next.setBounds (area.removeFromRight (28).withSizeKeepingCentre (28, 40));
    displayArea = area.reduced (6, 0);
}

void ModelSlot::mouseUp (const juce::MouseEvent& event)
{
    if (displayArea.contains (event.getPosition()))
        chooseFile();
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
