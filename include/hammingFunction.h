// Code from http://ofdsp.blogspot.com/2011/08/short-time-fourier-transform-with-fftw3.html

#include <Kernel/math.h>
#include <vector>

#ifndef HAMMINGFUNCTION_H
#define HAMMINGFUNCTION_H 

#define PI 3.14159265358979323846

void hammingFunction(int windowLength, double* buffer);

#endif // HAMMINGFUNCTION_H