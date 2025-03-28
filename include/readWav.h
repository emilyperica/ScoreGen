#ifndef READWAV_H
#define READWAV_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <string>

bool readWav(const std::string& filename, std::vector<double>& samples, int& sampleRate);

#endif // READWAV_H