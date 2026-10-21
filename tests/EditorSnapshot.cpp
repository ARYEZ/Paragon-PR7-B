#include <catch2/catch_test_macros.hpp>

#include "PluginEditor.h"
#include "PluginProcessor.h"

namespace
{
    juce::Component* findById (juce::Component& parent, const juce::String& id)
    {
        if (parent.getComponentID() == id)
            return &parent;
        for (auto* child : parent.getChildren())
            if (auto* found = findById (*child, id))
                return found;
        return nullptr;
    }
}

// Hidden test (the "[.]" tag): renders the editor to a PNG for reviewing the
// UI without a DAW. Run with:
//   PARAGON_SNAPSHOT=editor.png ./ParagonX1Tests "[snapshot]"
// Optionally set PARAGON_SNAPSHOT_MODEL and PARAGON_SNAPSHOT_IR to file paths
// to show the amp and cabinet slots loaded, and PARAGON_SNAPSHOT_EFFECTS=1 to
// switch on the drive, delay and reverb. PARAGON_SNAPSHOT_PAGE=presets shows
// the rig browser, with a couple of example user rigs. PARAGON_SNAPSHOT_WIDTH
// sets the window width (default 1280).
TEST_CASE ("Render the editor to a PNG", "[.][snapshot]")
{
    const auto path = juce::SystemStats::getEnvironmentVariable ("PARAGON_SNAPSHOT", "editor.png");

    ParagonX1AudioProcessor processor;
    processor.prepareToPlay (48000.0, 256);

    const auto showPresets = juce::SystemStats::getEnvironmentVariable ("PARAGON_SNAPSHOT_PAGE", {}) == "presets";
    const auto rigFolder = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("ParagonX1SnapshotRigs");
    rigFolder.deleteRecursively();
    processor.useRigFolder (rigFolder);

    if (const auto model = juce::SystemStats::getEnvironmentVariable ("PARAGON_SNAPSHOT_MODEL", {}); model.isNotEmpty())
        processor.loadAmpModel (juce::File (model));
    if (const auto ir = juce::SystemStats::getEnvironmentVariable ("PARAGON_SNAPSHOT_IR", {}); ir.isNotEmpty())
        processor.loadCabinetIR (juce::File (ir));
    REQUIRE (processor.waitForLoads (20000));

    if (juce::SystemStats::getEnvironmentVariable ("PARAGON_SNAPSHOT_EFFECTS", {}) == "1")
        for (const auto* id : { "driveOn", "delayOn", "reverbOn" })
            processor.getParameters().getParameter (id)->setValueNotifyingHost (1.0f);

    if (showPresets)
    {
        processor.saveCurrentRigAs ("Cyber Drive");
        processor.saveCurrentRigAs ("Sunday Worship Clean");
        processor.loadRig (*processor.getRigLibrary().find (rigFolder.getChildFile ("Cyber Drive.x1rig")));
        processor.getParameters().getParameter ("driveGain")->setValueNotifyingHost (0.8f);   // shows the * marker
    }

    std::unique_ptr<juce::AudioProcessorEditor> editor (processor.createEditor());
    if (showPresets)
        if (auto* tab = dynamic_cast<juce::Button*> (findById (*editor, "presetsTab")))
            tab->onClick();
    if (const auto width = juce::SystemStats::getEnvironmentVariable ("PARAGON_SNAPSHOT_WIDTH", {}).getIntValue(); width > 0)
        editor->setSize (width, width * 9 / 16);
    editor->resized();
    const auto image = editor->createComponentSnapshot (editor->getLocalBounds());

    juce::File file (juce::File::getCurrentWorkingDirectory().getChildFile (path));
    file.deleteFile();
    juce::FileOutputStream stream (file);
    REQUIRE (stream.openedOk());
    REQUIRE (juce::PNGImageFormat().writeImageToStream (image, stream));
}
