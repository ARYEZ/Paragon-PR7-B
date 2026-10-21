#include "PluginEditor.h"

ParagonX1AudioProcessorEditor::ParagonX1AudioProcessorEditor (ParagonX1AudioProcessor& processorToEdit)
    : AudioProcessorEditor (processorToEdit),
      view (processorToEdit)
{
    setLookAndFeel (&lookAndFeel);

    view.setSize (MainView::width, MainView::height);
    addAndMakeVisible (view);

    // Keep the mockup's 16:9 shape at any size the user drags to.
    setResizable (true, true);
    setResizeLimits (960, 540, 1920, 1080);
    getConstrainer()->setFixedAspectRatio ((double) MainView::width / (double) MainView::height);
    setSize (defaultWidth, defaultHeight);
}

ParagonX1AudioProcessorEditor::~ParagonX1AudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void ParagonX1AudioProcessorEditor::resized()
{
    // Scale the whole view rather than re-laying it out, so it looks the
    // same at every size. JUCE draws through the transform, so text and
    // shapes stay sharp.
    view.setTransform (juce::AffineTransform::scale ((float) getWidth() / (float) MainView::width));
}
