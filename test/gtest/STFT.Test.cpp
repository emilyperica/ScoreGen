#include <gtest/gtest.h>
#include "fftw3.h"
#include "test-helpers/test-helpers.h"

#define SAMPLE_RATE 44100.0
#define WINDOW_SIZE 2048
#define HOP_SIZE 441
#define FP_TOL 1e-6

TEST(STFTTests, SineWaveFrequency) {
    double frequency = 440.0; // A4
    double duration = 1.0;

    auto sineWave = generateSineWave(frequency, SAMPLE_RATE, duration);
    auto spectrogram = STFT(sineWave, WINDOW_SIZE, HOP_SIZE);
    auto detectedFreq = extractFundamentalFrequency(spectrogram, SAMPLE_RATE);

    // 10.0 Hz error tolerance, adjust to percentage (e.g. 0.1% of target frequency)
    EXPECT_NEAR(detectedFreq, frequency, 10.0);
}

TEST(STFTTests, SpectrogramDimensions) {
    double duration = 0.5;
    double val = 123.0;

    auto constSignal = generateConstantSignal(val, SAMPLE_RATE, duration);
    auto spectrogram = STFT(constSignal, WINDOW_SIZE, HOP_SIZE);
    int expectedFrames = static_cast<int>(ceil((duration * SAMPLE_RATE - WINDOW_SIZE) / HOP_SIZE)) + 1;
    EXPECT_EQ(spectrogram.size(), static_cast<size_t>(expectedFrames));

    // Number of freq bins should be windowSize / 2 + 1
    EXPECT_EQ(spectrogram[0].size(), static_cast<size_t>(WINDOW_SIZE / 2 + 1));
}

TEST(STFTTests, ConstantSignal) {
    double duration = 0.5;
    double val = 1.0;

    // Compute the FFT of a Hamming windowed constant signal
    auto constSignal = generateConstantSignal(val, SAMPLE_RATE, duration);
    auto spectrogram = STFT(constSignal, WINDOW_SIZE, HOP_SIZE);

    // Compute the FFT of Hamming window
    std::vector<double> hammingWindow = hammingFunction(WINDOW_SIZE);
    fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * WINDOW_SIZE);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * WINDOW_SIZE);
    fftw_plan planForward = fftw_plan_dft_1d(WINDOW_SIZE, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    for (int i = 0; i < WINDOW_SIZE; i++) {
        in[i][0] = val * hammingWindow[i];
        in[i][1] = 0.0;
    }
    fftw_execute(planForward);

    // Skip the last frame, padded with zeros by STFT operation
    for (int i = 0; i < spectrogram.size() - 1 ; i++) {
        for (int j = 0; j < spectrogram[i].size(); j++) {
            double expected = sqrt(out[j][0]*out[j][0] + out[j][1]*out[j][1]);
            EXPECT_NEAR(spectrogram[i][j], expected, FP_TOL);
        }
    }
}

TEST(STFTTests, EmptySignal) {
    std::vector<double> emptySignal;
    auto spectrogram = STFT(emptySignal, WINDOW_SIZE, HOP_SIZE);

    EXPECT_TRUE(spectrogram.empty());
}