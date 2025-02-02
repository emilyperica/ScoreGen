#ifndef COMMON_H
#define COMMON_H

#include <string>

struct xmlNote {
    std::string pitch; // Note pitch (e.g., "C")
    float alter = 0; // Chromatic alteration 
    int octave = 4; // Octave
    int duration = 0; // Duration in divisions
    std::string type; // Note type (i.e. quarter, half, etc.)
    bool isRest = false; // Rest note flag
};

#endif