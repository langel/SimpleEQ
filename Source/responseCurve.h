/*
  ==============================================================================

    responseCurve.h
    Created: 23 Jun 2021 8:48:16pm
    Author:  puke7

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "pathProducer.h"


struct ResponseCurveComponent : juce::Component,
	juce::AudioProcessorParameter::Listener,
	juce::Timer
{
	ResponseCurveComponent(SimpleEQAudioProcessor&);
	~ResponseCurveComponent();
	void parameterValueChanged(int parameterIndex, float newValue) override;
	void parameterGestureChanged(int paramaterIndex, bool gestureIsStarting) override { };

	void timerCallback() override;
	void paint(juce::Graphics& g) override;
	void resized() override;
	void toggleAnalysisEnablement(bool enabled) {
		shouldShowFFTAnalysis = enabled;
	}

private:
	SimpleEQAudioProcessor& audioProcessor;
	juce::Atomic<bool> parametersChanged{ false };
	MonoChain monoChain;
	void updateChain();
	juce::Image background;
	juce::Rectangle<int> getRenderArea();
	juce::Rectangle<int> getAnalysisArea();
	PathProducer leftPathProducer, rightPathProducer;
	bool shouldShowFFTAnalysis = true;
};