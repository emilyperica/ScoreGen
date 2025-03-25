#ifndef NOTE_DURATION_EXTRACTOR_H
#define NOTE_DURATION_EXTRACTOR_H

#include <string>
#include <vector>
#include "common.h"

// Processes the input WAV file and extracts note durations.
// Returns a vector of Note objects with start time, end time, pitch, and note type.
std::vector<Note> extract_note_durations(const char* infilename, int bpm);

#endif // NOTE_DURATION_EXTRACTOR_H
