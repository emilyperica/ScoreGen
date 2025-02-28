#define _USE_MATH_DEFINES

#include <vector>
#include <cmath>
#include <string>

#include "STFT.h"
#include "hammingFunction.h"

// STFT helpers
std::vector<double> generateSineWave(double frequency, double sampleRate, double duration);
std::vector<double> generateConstantSignal(double value, double sampleRate, double duration);
float extractFundamentalFrequency(const std::vector<std::vector<double>>& spectrogram, double sampleRate);