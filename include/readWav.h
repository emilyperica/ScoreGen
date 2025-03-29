#ifndef READWAV_H
#define READWAV_H

#include <vector>
#include <fstream>
#include <iostream>

bool readWav(const std::string& filename, std::vector<double>& samples, int& sampleRate);

#endif // READWAV_H