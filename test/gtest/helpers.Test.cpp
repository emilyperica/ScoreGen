#include <gtest/gtest.h>
#include "test-helpers/test-helpers.h"

#define SAMPLE_RATE 44100.0
#define WINDOW_SIZE 2048
#define HOP_SIZE 441
#define FP_TOL 1e-6

// Tests for generateSineWave function
TEST(SineWaveTest, SignalSize) {
    double frequency = 440.0;
    double sampleRate = 44100.0;
    double duration = 1.0;
    auto signal = generateSineWave(frequency, sampleRate, duration);
    EXPECT_EQ(signal.size(), static_cast<size_t>(sampleRate * duration));
}

TEST(SineWaveTest, AmplitudeConsistency) {
    double frequency = 440.0;
    double sampleRate = 44100.0;
    double duration = 1.0;
    auto signal = generateSineWave(frequency, sampleRate, duration);
    for (const auto& sample : signal) {
        EXPECT_LE(sample, 1.0);
        EXPECT_GE(sample, -1.0);
    }
}

TEST(SineWaveTest, PhaseContinuity) {
    double frequency = 440.0;
    double sampleRate = 44100.0;
    double duration = 1.0;
    auto signal = generateSineWave(frequency, sampleRate, duration);
    EXPECT_NEAR(signal[0], 0.0, 1e-6);
}

TEST(SineWaveTest, ZeroDuration) {
    double frequency = 440.0;
    double sampleRate = 44100.0;
    double duration = 0.0;
    auto signal = generateSineWave(frequency, sampleRate, duration);
    EXPECT_TRUE(signal.empty());
}

TEST(SineWaveTest, HighFrequency) {
    double frequency = 44100.0/2.0; // Full cycle over 2 sampling intervals, should be zeros
    double sampleRate = 44100.0;
    double duration = 1.0;
    auto signal = generateSineWave(frequency, sampleRate, duration);
    for (auto& sample : signal) {
        EXPECT_NEAR(sample, 0.0, 1e-6);
    }
}

// Tests for extractFundamentalFrequency function
TEST(FundamentalFrequencyTest, SingleFramePeak) {
    int numBins = WINDOW_SIZE / 2 + 1;
    int maxBin = 100;
    std::vector<std::vector<double>> spectrogram(1, std::vector<double>(numBins, 0.0));
    
    spectrogram[0][maxBin] = 1.0;
    
    float resolution = SAMPLE_RATE / (numBins * 2 - 1);
    float expectedFrequency = resolution * maxBin;
    
    float frequency = extractFundamentalFrequency(spectrogram, SAMPLE_RATE);
    
    EXPECT_NEAR(frequency, expectedFrequency, FP_TOL);
}

TEST(FundamentalFrequencyTest, MultiFrameCompetingPeaks) {
    int numBins = WINDOW_SIZE / 2 + 1;
    std::vector<std::vector<double>> spectrogram(2, std::vector<double>(numBins, 0.0));
    
    spectrogram[0][50] = 1.0;
    spectrogram[1][75] = 2.0;
    
    float freqResolution = SAMPLE_RATE / (numBins * 2 - 1);
    float expectedFrequency = freqResolution * 75;
    
    float frequency = extractFundamentalFrequency(spectrogram, SAMPLE_RATE);
    EXPECT_NEAR(frequency, expectedFrequency, FP_TOL);
}

TEST(FundamentalFrequencyTest, EmptySpectrogram) {
    std::vector<std::vector<double>> emptySpectrogram;
    
    float frequency = extractFundamentalFrequency(emptySpectrogram, SAMPLE_RATE);
    EXPECT_EQ(frequency, 0.0f);
}

TEST(FundamentalFrequencyTest, DifferentSampleRate) {
    int numBins = 1024;
    int maxBin = 100;
    double sampleRate = 16000.0;
    std::vector<std::vector<double>> spectrogram(1, std::vector<double>(numBins, 0.0));
    
    spectrogram[0][maxBin] = 1.0;
    
    float resolution = sampleRate / (numBins * 2 - 1);
    float expectedFrequency = resolution * maxBin;
    float frequency = extractFundamentalFrequency(spectrogram, sampleRate);
    EXPECT_NEAR(frequency, expectedFrequency, FP_TOL);
}