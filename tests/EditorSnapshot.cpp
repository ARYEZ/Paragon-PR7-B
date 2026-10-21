#include <catch2/catch_test_macros.hpp>

#include "PluginEditor.h"
#include "PluginProcessor.h"

// Hidden test (the "[.]" tag): renders the editor to a PNG for reviewing the
// UI without a DAW. Run with:
//   PARAGON_SNAPSHOT=editor.png ./ParagonX1Tests "[snapshot]"
// Optionally set PARAGON_SNAPSHOT_MODEL and PARAGON_SNAPSHOT_IR to file paths
// to show the amp and cabinet slots loaded.
TEST_CASE ("Render the editor to a PNG", "[.][snapshot]")
{
    const auto path = juce::SystemStats::getEnvironmentVariable ("PARAGON_SNAPSHOT", "editor.png");

    ParagonX1AudioProcessor processor;
    processor.prepareToPlay (48000.0, 256);

    if (const auto model = juce::SystemStats::getEnvironmentVariable ("PARAGON_SNAPSHOT_MODEL", {}); model.isNotEmpty())
        processor.loadAmpModel (juce::File (model));
    if (const auto ir = juce::SystemStats::getEnvironmentVariable ("PARAGON_SNAPSHOT_IR", {}); ir.isNotEmpty())
        processor.loadCabinetIR (juce::File (ir));
    REQUIRE (processor.waitForLoads (20000));

    std::unique_ptr<juce::AudioProcessorEditor> editor (processor.createEditor());
    const auto image = editor->createComponentSnapshot (editor->getLocalBounds());

    juce::File file (juce::File::getCurrentWorkingDirectory().getChildFile (path));
    file.deleteFile();
    juce::FileOutputStream stream (file);
    REQUIRE (stream.openedOk());
    REQUIRE (juce::PNGImageFormat().writeImageToStream (image, stream));
}
