#include <vector>
#include <string>

#ifndef FINDKEY_H
#define FINDKEY_H

float getCorrelation(float x_hat, float y_hat, std::vector<float> x, std::vector<int> y);
std::string findKey(std::vector<int> durations);

#endif