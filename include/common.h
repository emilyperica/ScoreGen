#ifndef COMMON_H
#define COMMON_H

#include <string>

struct XMLNote {
    std::string pitch; // Note pitch (e.g., "C")
    int alter = 0; // Chromatic alteration 
    int octave; // Octave
    int duration; // Duration in divisions
    std::string type; // Note type (i.e. quarter, half, etc.)
    bool isRest = false; // Rest note flag
};

struct DSPResult {
    std::vector<XMLNote> XMLNotes;
    std::string timeSignature;
    int keySignature;
    int bpm;
    int divisions;
};

struct Note {
    float startTime;
    float endTime;
    std::string pitch;
    std::string type;   // Note type (e.g., "quarter", "eighth")
};

#endif