#ifndef COMMON_H
#define COMMON_H

#include <string>

enum class Staff { Treble, Bass };

struct XMLNote {
    std::string pitch;   // Note pitch letter (e.g., "C")
    int alter = 0;       // Chromatic alteration (-1 = flat, 1 = sharp)
    int octave;          // Octave number (C4 = middle C)

    int startTime;       // Start time in divisions (relative to measure)
    int duration;        // Duration in divisions
    std::string type;    // Note type (quarter, half, etc.)

    bool isRest = false; // Whether this is a rest
    bool isChord = false; // Whether this belongs to a chord
    int voice = 1;       // Voice number (for polyphony)
    Staff staff;         // Assigned staff (treble or bass)
};

struct XMLMeasure {
    int divisions;                   // Divisions per quarter note
    std::vector<XMLNote> notes;       // All notes in the measure (sorted by startTime)
};


#endif