// Code adapted from http://ofdsp.blogspot.com/2011/08/short-time-fourier-transform-with-fftw3.html

#ifndef STFT_H
#define STFT_H

#include <fftw3.h>
#include <vector>

std::vector<std::vector<double>> STFT(const std::vector<double>& data, int windowLength, int hopSize);

#endif // STFT_H