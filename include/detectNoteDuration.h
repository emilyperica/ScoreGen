#ifndef DETECT_NOTE_DURATION_H
#define DETECT_NOTE_DURATION_H

#include <vector>
#include <string>
#include <utility> // For std::pair

// Structure to store detected note information
struct Note {
    double start_time;  // in seconds
    double end_time;    // in seconds
    std::string length; // Note length (Whole, Half, Quarter, etc.)
};

// Function to assign note lengths based on time signature and BPM
std::vector<std::pair<std::string, double>> detectNoteDuration(float* buf, int numFrames, int sampleRate, int channels, double tempo);

#endif // DETECT_NOTE_DURATION_H
