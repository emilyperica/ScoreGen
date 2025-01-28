#ifndef COMMON_H
#define COMMON_H

#include <string>

using namespace std;

struct Note {
    string pitch; // Note pitch (e.g., "C")
    float alter = 0; // Chromatic alteration 
    int octave = 4; // Octave
    int duration = 0; // Duration in divisions
    string type; // Note type (i.e. quarter, half, etc.)
    bool isRest = false; // Rest note flag
};

#endif