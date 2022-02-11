/*
 // Copyright (c) 2015-2021 Pierre Guillot and Timothy Schoen.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#pragma once

#define JUCE_DISPLAY_SPLASH_SCREEN 0

#define JucePlugin_LV2URI (juce::String("urn:PlugData:") + juce::String("PlugData")).toUTF8()

namespace ProjectInfo
{
const char* const companyName = "Octagon";
const char* const versionString = "0.3.2";
}  // namespace ProjectInfo
