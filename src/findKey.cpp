#include <iostream>
#include <numeric>
#include <vector>
#include <math.h>
#include <string>
#include <utility>
#include "dsp.h"

/* Returns a key prediction by using the Krumhansl-Schumckler key-finding algorithm as 
described at http://rnhart.net/articles/key-finding/ */

float getCorrelation(float x_hat, float y_hat, std::vector<float> x, std::vector<int> y) {
    float corr;
    float num=0, den_a=0, den_b=0;
    for (int i=0; i<12; i++) {
        num = num + ((x[i] - x_hat)*(y[i] - y_hat));
        den_a = den_a + (float) pow((x[i] - x_hat), 2.0f);
        den_b = den_b + (float) pow((y[i] - y_hat), 2.0f);
    }

    corr = num / sqrt(den_a * den_b);
    return corr;
}

std::string findKey(std::vector<int> durations)
{
    float minor_coeff;
    float major_coeff;
    std::pair<std::string, float> key("C", 0.0f);
    std::vector<std::string> const major_keys{"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    std::vector<std::string> const minor_keys{"c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"};
    std::vector<float> const major_prof{6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88};
    std::vector<float> const minor_prof{6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17};
    float major_avg = std::accumulate(major_prof.begin(), major_prof.end(), 0.0)/12.0; // x_hat
    float minor_avg = std::accumulate(minor_prof.begin(), minor_prof.end(), 0.0)/12.0; // x_hat
    float duration_avg = std::accumulate(durations.begin(), durations.end(), 0.0)/12.0;; // y_hat

    for (int i=0; i<12; i++) {
        std::vector<int> key_sig;
        if (i == 0) {
            key_sig = durations;
        }
        else {
            key_sig.assign(durations.begin() + i, durations.end());
            key_sig.insert(key_sig.end(), durations.begin(), durations.begin() + i);
        }

        minor_coeff = getCorrelation(minor_avg, duration_avg, minor_prof, key_sig);
        major_coeff = getCorrelation(major_avg, duration_avg, major_prof, key_sig);
        if (minor_coeff > key.second) key = {minor_keys[i], minor_coeff};
        if (major_coeff > key.second) key = {major_keys[i], major_coeff};
    }

    return key.first;
}


