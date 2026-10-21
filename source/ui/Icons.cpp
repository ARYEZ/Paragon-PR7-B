#include "Icons.h"

namespace Icons
{
    namespace
    {
        // A pulse line, like a heartbeat: used for compressor and reverb shapes.
        void addPulse (juce::Path& p, std::initializer_list<juce::Point<float>> points)
        {
            bool first = true;
            for (auto point : points)
            {
                if (first)
                    p.startNewSubPath (point);
                else
                    p.lineTo (point);
                first = false;
            }
        }
    }

    juce::Path get (Type type)
    {
        juce::Path p;

        switch (type)
        {
            case Type::amp:
                // Amp head: cabinet outline, grille slots, a row of knobs.
                p.addRoundedRectangle (0.05f, 0.22f, 0.9f, 0.56f, 0.06f);
                p.addRectangle (0.12f, 0.32f, 0.76f, 0.001f);
                for (int i = 0; i < 6; ++i)
                    p.addRectangle (0.16f + (float) i * 0.12f, 0.40f, 0.06f, 0.16f);
                for (int i = 0; i < 5; ++i)
                    p.addEllipse (0.2f + (float) i * 0.14f, 0.62f, 0.05f, 0.05f);
                break;

            case Type::cabinet:
                p.addRoundedRectangle (0.12f, 0.08f, 0.76f, 0.84f, 0.06f);
                p.addEllipse (0.24f, 0.2f, 0.52f, 0.52f);
                p.addEllipse (0.42f, 0.38f, 0.16f, 0.16f);
                p.addEllipse (0.46f, 0.78f, 0.08f, 0.08f);
                break;

            case Type::rig:
                // An isometric cube.
                p.startNewSubPath (0.5f, 0.08f);
                p.lineTo (0.88f, 0.28f);
                p.lineTo (0.88f, 0.72f);
                p.lineTo (0.5f, 0.92f);
                p.lineTo (0.12f, 0.72f);
                p.lineTo (0.12f, 0.28f);
                p.closeSubPath();
                p.startNewSubPath (0.12f, 0.28f);
                p.lineTo (0.5f, 0.48f);
                p.lineTo (0.88f, 0.28f);
                p.startNewSubPath (0.5f, 0.48f);
                p.lineTo (0.5f, 0.92f);
                break;

            case Type::comp:
                addPulse (p, { { 0.0f, 0.5f }, { 0.3f, 0.5f }, { 0.38f, 0.3f }, { 0.46f, 0.82f },
                               { 0.56f, 0.12f }, { 0.64f, 0.66f }, { 0.7f, 0.5f }, { 1.0f, 0.5f } });
                break;

            case Type::drive:
                // A stompbox: body, two knobs, footswitch.
                p.addRoundedRectangle (0.22f, 0.06f, 0.56f, 0.88f, 0.08f);
                p.addEllipse (0.32f, 0.16f, 0.12f, 0.12f);
                p.addEllipse (0.56f, 0.16f, 0.12f, 0.12f);
                p.addEllipse (0.4f, 0.6f, 0.2f, 0.2f);
                break;

            case Type::modulation:
                p.startNewSubPath (0.0f, 0.5f);
                for (int i = 1; i <= 40; ++i)
                {
                    const auto x = (float) i / 40.0f;
                    p.lineTo (x, 0.5f - 0.36f * std::sin (x * juce::MathConstants<float>::twoPi * 2.0f));
                }
                break;

            case Type::delay:
                // Repeats fading round a circle.
                for (int i = 0; i < 9; ++i)
                {
                    const auto angle = (float) i / 9.0f * juce::MathConstants<float>::twoPi * 0.85f;
                    const auto r = 0.05f + 0.06f * (float) (9 - i) / 9.0f;
                    p.addEllipse (0.5f + 0.38f * std::sin (angle) - r, 0.5f - 0.38f * std::cos (angle) - r, r * 2.0f, r * 2.0f);
                }
                break;

            case Type::reverb:
                addPulse (p, { { 0.0f, 0.5f }, { 0.2f, 0.5f }, { 0.26f, 0.05f }, { 0.32f, 0.95f }, { 0.4f, 0.15f },
                               { 0.48f, 0.85f }, { 0.56f, 0.28f }, { 0.64f, 0.72f }, { 0.72f, 0.4f }, { 0.8f, 0.6f },
                               { 0.86f, 0.5f }, { 1.0f, 0.5f } });
                break;

            case Type::guitar:
                // Body and neck of an electric guitar, angled.
                p.addEllipse (0.06f, 0.52f, 0.3f, 0.3f);
                p.addEllipse (0.2f, 0.42f, 0.24f, 0.24f);
                p.startNewSubPath (0.34f, 0.56f);
                p.lineTo (0.86f, 0.1f);
                p.addRectangle (0.8f, 0.04f, 0.14f, 0.1f);
                break;

            case Type::speaker:
                p.startNewSubPath (0.08f, 0.38f);
                p.lineTo (0.24f, 0.38f);
                p.lineTo (0.44f, 0.18f);
                p.lineTo (0.44f, 0.82f);
                p.lineTo (0.24f, 0.62f);
                p.lineTo (0.08f, 0.62f);
                p.closeSubPath();
                p.addCentredArc (0.44f, 0.5f, 0.18f, 0.18f, 0.0f, 0.5f, 2.64f, true);
                p.addCentredArc (0.44f, 0.5f, 0.32f, 0.32f, 0.0f, 0.55f, 2.6f, true);
                p.addCentredArc (0.44f, 0.5f, 0.46f, 0.46f, 0.0f, 0.6f, 2.55f, true);
                break;
        }

        return p;
    }

    void draw (juce::Graphics& g, Type type, juce::Rectangle<float> area, juce::Colour colour, float strokeWidth, bool glow)
    {
        const auto size = juce::jmin (area.getWidth(), area.getHeight());
        auto path = get (type);
        path.applyTransform (juce::AffineTransform::scale (size).translated (area.getCentreX() - size / 2.0f,
                                                                                  area.getCentreY() - size / 2.0f));

        if (glow)
        {
            g.setColour (colour.withAlpha (0.25f));
            g.strokePath (path, juce::PathStrokeType (strokeWidth * 3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        g.setColour (colour);
        g.strokePath (path, juce::PathStrokeType (strokeWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}
