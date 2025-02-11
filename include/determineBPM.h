#ifndef DETERMINE_BPM_H
#define DETERMINE_BPM_H

#define WIN_S 1024
#define HOP_S 512
#define F_WIN_S 512
#define F_HOP_S 128
#define SF_WIN_S 128
#define SF_HOP_S 64

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>
#include <aubio/aubio.h>

float calculateMedian(const std::vector<float>& values);
float getBufferBPM(const std::vector<float>& buf, int sample_rate, const std::map<std::string, std::string>& params = {});
#endif // DETERMINE_BPM_H
