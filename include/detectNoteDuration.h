#ifndef NOTE_DETECTION_H
#define NOTE_DETECTION_H

#include <vector>
#include <string>
#include <cmath>
#include <map>
#include <iostream>
#include "aubio/aubio.h"

// Struct to represent a detected note
struct Note {
    float startTime;
    float endTime;
    std::string pitch;
    std::string type;   // Note type (e.g., "quarter", "eighth")
};

// Function declarations
std::string getNoteName(double freq);
std::string determineNoteType(float noteDuration, int bpm);
std::vector<Note> detectNotes(const std::vector<float>& buf, int sample_rate, int channels, int bpm);

#endif // NOTE_DETECTION_H
