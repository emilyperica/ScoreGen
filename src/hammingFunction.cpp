#include "hammingFunction.h"

void hammingFunction(int windowLength, float* buffer){

    for (int i = 0; i < windowLength; i++){
        buffer[i] *= 0.54 - 0.46 * cos(2 * PI * i / (windowLength - 1));
    }
}