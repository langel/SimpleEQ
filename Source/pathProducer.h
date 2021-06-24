/*
  ==============================================================================

    pathProducer.h
    Created: 23 Jun 2021 8:57:50pm
    Author:  puke7

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "fft.h"
#include "analyzer.h"


struct PathProducer {

	PathProducer(SingleChannelSampleFifo<SimpleEQAudioProcessor::BlockType>& scsf) : leftChannelFifo(&scsf) {
		leftChannelFFTDataGenerator.changeOrder(FFTOrder::order2048);
		monoBuffer.setSize(1, leftChannelFFTDataGenerator.getFFTSize());
	}

	juce::Path getPath() { return leftChannelFFTPath; };

	void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate) {
		juce::AudioBuffer<float> tempIncomingBuffer;
		while (leftChannelFifo->getNumCompleteBuffersAvailable() > 0) {
			if (leftChannelFifo->getAudioBuffer(tempIncomingBuffer)) {
				auto size = tempIncomingBuffer.getNumSamples();
				juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0),
					monoBuffer.getReadPointer(0, size),
					monoBuffer.getNumSamples() - size);
				juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
					tempIncomingBuffer.getReadPointer(0, 0),
					size);
				leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
			}
		}
		// if there are FFT data buffers to pull
			// if we can pull a buffer
				// generate a path
		const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();

		const auto binWdith = sampleRate / (double)fftSize;

		while (leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0) {
			std::vector<float> fftData;
			if (leftChannelFFTDataGenerator.getFFTData(fftData)) {
				pathProducer.generatePath(fftData, fftBounds, fftSize, binWdith, -48.f);
			}
		}

		// pull all paths and display most recent one
		while (pathProducer.getNumPathsAvailable()) {
			pathProducer.getPath(leftChannelFFTPath);
		}
	}


private:
	SingleChannelSampleFifo<SimpleEQAudioProcessor::BlockType>* leftChannelFifo;
	juce::AudioBuffer<float> monoBuffer;
	FFTDataGenerator<std::vector<float>> leftChannelFFTDataGenerator;
	AnalyzerPathGenerator<juce::Path> pathProducer;
	juce::Path leftChannelFFTPath;
};

