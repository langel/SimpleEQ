/*
  ==============================================================================

    rotarySlider.h
    Created: 23 Jun 2021 8:18:30pm
    Author:  puke7

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "lookAndFeel.h"

struct RotarySliderWithLabels : juce::Slider {

	RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix) :
		juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
		param(&rap),
		suffix(unitSuffix)
	{
		setLookAndFeel(&lnf);
	}

	~RotarySliderWithLabels() {
		setLookAndFeel(nullptr);
	}

	struct LabelPos {
		float pos;
		juce::String label;
	};

	juce::Array<LabelPos> labels;

	void paint(juce::Graphics& g) override;
	int getTextHeight() const { return 14; }
	juce::String getDisplayString() const;
	juce::Rectangle<int> getSliderBounds() const;

private:
	LookAndFeel lnf;
	juce::RangedAudioParameter* param;
	juce::String suffix;
};