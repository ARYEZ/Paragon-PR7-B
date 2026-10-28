#pragma once

#include "Rig.h"

#include <vector>

// Starter sounds that ship with the plugin. They set only the effects and
// keep the user's own amp and cabinet (see Rig::keepAmpAndCab).
std::vector<Rig> makeFactoryRigs();

// A factory rig's name today, for a name saved before it was renamed
// (other names come back unchanged).
juce::String currentFactoryName (const juce::String& name);
