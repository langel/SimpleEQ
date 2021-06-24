/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "lookAndFeel.h"
#include "responseCurve.h"
#include "rotarySlider.h"






//==============================================================================


/**
*/
class SimpleEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleEQAudioProcessor& audioProcessor;

	RotarySliderWithLabels peakFreqSlider, peakGainSlider, peakQualitySlider, lowCutFreqSlider, highCutFreqSlider;
	RotarySliderWithLabels lowCutSlopeSlider, highCutSlopeSlider;

	ResponseCurveComponent responseCurveComponent;

	using APVTS = juce::AudioProcessorValueTreeState;
	using Attachment = APVTS::SliderAttachment;

	Attachment peakFreqSliderAttachment, peakGainSliderAttachment, peakQualitySliderAttachment, lowCutFreqSliderAttachment, highCutFreqSliderAttachment;
	Attachment lowCutSlopeSliderAttachment, highCutSlopeSliderAttachment;

	PowerButton lowcutBypassButton, peakBypassButton, highcutBypassButton;
	AnalyzerButton analyzerEnabledButton;

	using ButtonAttachment = APVTS::ButtonAttachment;
	ButtonAttachment lowcutBypassButtonAttachment, peakBypassButtonAttachment, highcutBypassButtonAttachment, analyzerEnabledButtonAttachment;

	std::vector<juce::Component*> getComps();

	LookAndFeel lnf;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleEQAudioProcessorEditor);
};
