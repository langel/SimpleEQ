/*
  ==============================================================================

    responseCurve.cpp
    Created: 23 Jun 2021 8:48:16pm
    Author:  puke7

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "responseCurve.h"

ResponseCurveComponent::ResponseCurveComponent(SimpleEQAudioProcessor& p) :
	audioProcessor(p),
	//leftChannelFifo(&audioProcessor.leftChannelFifo)
	leftPathProducer(audioProcessor.leftChannelFifo),
	rightPathProducer(audioProcessor.rightChannelFifo)
{
	const auto& params = audioProcessor.getParameters();
	for (auto param : params) {
		param->addListener(this);
	}


	updateChain();
	startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent() {
	const auto& params = audioProcessor.getParameters();
	for (auto param : params) {
		param->removeListener(this);
	}

}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue) {
	parametersChanged.set(true);
}



void ResponseCurveComponent::timerCallback() {

	if (shouldShowFFTAnalysis) {
		auto fftBounds = getAnalysisArea().toFloat();
		auto sampleRate = audioProcessor.getSampleRate();
		leftPathProducer.process(fftBounds, sampleRate);
		rightPathProducer.process(fftBounds, sampleRate);
	}

	if (parametersChanged.compareAndSetBool(false, true)) {
		DBG("params changed");
		// update monochain
		updateChain();
	}
	// signal repaint
	repaint();
}

void ResponseCurveComponent::updateChain() {
	auto chainSettings = getChainSettings(audioProcessor.apvts);

	monoChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
	monoChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
	monoChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);

	auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
	updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);

	auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
	auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());

	updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
	updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
}

void ResponseCurveComponent::paint(juce::Graphics& g) {
	using namespace juce;
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(Colours::black);

	g.drawImage(background, getLocalBounds().toFloat());

	auto responseArea = getAnalysisArea();

	auto w = responseArea.getWidth();

	auto& lowcut = monoChain.get<ChainPositions::LowCut>();
	auto& peak = monoChain.get<ChainPositions::Peak>();
	auto& highcut = monoChain.get<ChainPositions::HighCut>();

	auto sampleRate = audioProcessor.getSampleRate();

	std::vector <double> mags;
	mags.resize(w);
	for (int i = 0; i < w; ++i) {
		double mag = 1.f;
		auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);

		if (!monoChain.isBypassed<ChainPositions::Peak>())
			mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);

		if (!monoChain.isBypassed<ChainPositions::LowCut>()) {
			if (!lowcut.isBypassed<0>())
				mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!lowcut.isBypassed<1>())
				mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!lowcut.isBypassed<2>())
				mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!lowcut.isBypassed<3>())
				mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}

		if (!monoChain.isBypassed<ChainPositions::HighCut>()) {
			if (!highcut.isBypassed<0>())
				mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!highcut.isBypassed<1>())
				mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!highcut.isBypassed<2>())
				mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!highcut.isBypassed<3>())
				mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}

		mags[i] = Decibels::gainToDecibels(mag);
	}

	Path responseCurve;

	const double outputMin = responseArea.getBottom();
	const double outputMax = responseArea.getY();
	auto map = [outputMin, outputMax](double input) {
		return jmap(input, -24.0, 24.0, outputMin, outputMax);
	};

	responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

	for (size_t i = 1; i < mags.size(); ++i) {
		responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
	}

	if (shouldShowFFTAnalysis) {
		auto leftChannelFFTPath = leftPathProducer.getPath();
		leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
		g.setColour(Colours::skyblue);
		g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));
		auto rightChannelFFTPath = rightPathProducer.getPath();
		rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
		g.setColour(Colours::lightyellow);
		g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
	}

	g.setColour(Colours::orange);
	g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);

	g.setColour(Colours::white);
	g.strokePath(responseCurve, PathStrokeType(2.f));
}

void ResponseCurveComponent::resized() {
	using namespace juce;
	background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);
	Graphics g(background);
	auto renderArea = getAnalysisArea();
	auto left = renderArea.getX();
	auto right = renderArea.getRight();
	auto top = renderArea.getY();
	auto bottom = renderArea.getBottom();
	auto width = renderArea.getWidth();
	Array<float> freqs{
		20, /*30, 40,*/ 50, 100,
		200, /*300, 400,*/ 500, 1000,
		2000, /*3000, 4000,*/ 5000, 10000,
		20000
	};
	Array<float> xs;
	for (auto f : freqs) {
		auto normX = mapFromLog10(f, 20.f, 20000.f);
		xs.add(left + width * normX);
	}
	g.setColour(Colours::dimgrey);
	for (auto x : xs) {
		g.drawVerticalLine(x, top, bottom);
	}
	Array<float> gain{
		-24, -12, 0, 12, 24
	};
	for (auto gDb : gain) {
		auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
		g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::darkgrey);
		g.drawHorizontalLine(y, left, right);
	}
	g.setColour(Colours::lightgrey);
	const int fontHeight = 10;
	g.setFont(fontHeight);
	for (int i = 0; i < freqs.size(); ++i) {
		auto f = freqs[i];
		auto x = xs[i];
		bool addK = false;
		String str;
		if (f > 999.f) {
			addK = true;
			f /= 1000.f;
		}
		str << f;
		if (addK)
			str << "k";
		str << "Hz";

		auto textWidth = g.getCurrentFont().getStringWidth(str);
		Rectangle<int> r;
		r.setSize(textWidth, fontHeight);
		r.setCentre(x, 0);
		r.setY(1);
		g.drawFittedText(str, r, juce::Justification::centred, 1);
	}
	for (auto gDb : gain) {
		auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
		String str;
		if (gDb > 0)
			str << "+";
		str << gDb;
		auto textWidth = g.getCurrentFont().getStringWidth(str);
		Rectangle<int> r;
		r.setSize(textWidth, fontHeight);
		r.setX(getWidth() - textWidth);
		r.setCentre(r.getCentreX(), y);
		g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::lightgrey);
		g.drawFittedText(str, r, juce::Justification::centred, 1);

		str.clear();
		str << (gDb - 24.f);
		r.setX(1);
		textWidth = g.getCurrentFont().getStringWidth(str);
		r.setSize(textWidth, fontHeight);
		g.setColour(Colours::lightgrey);
		g.drawFittedText(str, r, juce::Justification::centred, 1);
	}
}

juce::Rectangle<int> ResponseCurveComponent::getRenderArea() {
	auto bounds = getLocalBounds();
	//bounds.reduce(10, 8);
	bounds.removeFromTop(12);
	bounds.removeFromBottom(2);
	bounds.removeFromLeft(20);
	bounds.removeFromRight(20);
	return bounds;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea() {
	auto bounds = getRenderArea();
	bounds.removeFromTop(4);
	bounds.removeFromBottom(4);
	return bounds;
}