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

struct Note {
    float startTime;   // Note start time in seconds.
    float endTime;     // Note end time in seconds.
    std::string pitch; // Note pitch (e.g., "A4", "C#3", or "Rest").
    std::string type;  // Note type (e.g., "quarter", "eighth", etc.)
};

#endif