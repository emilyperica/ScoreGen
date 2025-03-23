#ifndef NOTE_DURATION_EXTRACTOR_H
#define NOTE_DURATION_EXTRACTOR_H

#include <string>
#include <vector>
#include "common.h"

// Processes the input WAV file and extracts note durations.
// Returns a vector of Note objects with start time, end time, pitch, and note type.
std::vector<Note> onsetDetection(const char* infilename, int sample_rate, int bpm);

#endif // NOTE_DURATION_EXTRACTOR_H
