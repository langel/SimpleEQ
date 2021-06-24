/*
  ==============================================================================

    filterChain.cpp
    Created: 23 Jun 2021 7:48:34pm
    Author:  puke7

  ==============================================================================
*/

#include "filterChain.h"


ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts) {
	ChainSettings settings;
	settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
	settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
	settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
	settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
	settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
	settings.lowCutSlope = static_cast<Slope>(int(apvts.getRawParameterValue("LowCut Slope")->load()));
	settings.highCutSlope = static_cast<Slope>(int(apvts.getRawParameterValue("HighCut Slope")->load()));
	settings.lowCutBypassed = apvts.getRawParameterValue("LowCut Bypassed")->load() > 0.5;
	settings.peakBypassed = apvts.getRawParameterValue("Peak Bypassed")->load() > 0.5;
	settings.highCutBypassed = apvts.getRawParameterValue("HighCut Bypassed")->load() > 0.5;
	return settings;
}


Coefficients makePeakFilter(const ChainSettings& chainSettings, double sampleRate) {
	return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
		chainSettings.peakFreq,
		chainSettings.peakQuality,
		juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));
}


void updateCoefficients(Coefficients& old, const Coefficients& replacements) {
	*old = *replacements;
}