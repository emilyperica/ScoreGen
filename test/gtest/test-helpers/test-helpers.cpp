#include "test-helpers.h"

std::vector<double> generateSineWave(double frequency, double sampleRate, double duration) {
    int numSamples = static_cast<int>(sampleRate * duration);
    std::vector<double> signal(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        // for continuous sine wave, sin(2wt)
        signal[i] = sin(2.0 * M_PI * frequency * i / sampleRate);
    }
    return signal;
}

std::vector<double> generateConstantSignal(double value, double sampleRate, double duration) {
    int numSamples = static_cast<int>(sampleRate * duration);
    return std::vector<double>(numSamples, value);
}

float extractFundamentalFrequency(const std::vector<std::vector<double>>& spectrogram, double sampleRate) {
    if (spectrogram.empty() || spectrogram[0].empty()) {
        return 0;
    }
    int numBins = spectrogram[0].size();
    float resolution = sampleRate / (numBins * 2 - 1);

    int fundamentalBin = -1;
    float maxMagnitude = 0.0f;

    for (const auto& frame : spectrogram) {
        for (int bin = 0; bin < numBins; ++bin) {
            if (frame[bin] > maxMagnitude) {
                maxMagnitude = frame[bin];
                fundamentalBin = bin;
            }
        }
    }

    return resolution * fundamentalBin;
}