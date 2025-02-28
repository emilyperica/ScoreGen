#define _USE_MATH_DEFINES
#include "hammingFunction.h"
#include <cmath>
#include <stdexcept>

std::vector<double> hammingFunction(int windowSize) {
    if (windowSize < 2) {
        throw std::invalid_argument("Error: windowSize must be >= 2");
    }
    std::vector<double> window(windowSize);
    for (int i = 0; i < windowSize; i++) {
        window[i] = 0.54 - 0.46 * cos((2 * M_PI * i) / (windowSize - 1));
    }
    return window;
}