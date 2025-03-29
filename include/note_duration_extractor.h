#ifndef NOTE_DURATION_EXTRACTOR_H
#define NOTE_DURATION_EXTRACTOR_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <map>
#include "common.h"
#include "readWav.h"
#include "hanningFunction.h"

// Processes the input WAV file and extracts note durations.
// Returns a vector of Note objects with start time, end time, pitch, and note type.
std::vector<Note> extract_note_durations(const char* infilename, int bpm);

#endif // NOTE_DURATION_EXTRACTOR_H
