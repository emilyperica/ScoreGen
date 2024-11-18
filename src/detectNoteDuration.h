#ifndef NOTE_DETECTION_H
#define NOTE_DETECTION_H

#include <vector>
#include <iostream>
#include <sndfile.h>

using namespace std;

// Function prototype for note detection
void detect_notes(SNDFILE* infile, SF_INFO& sfinfo, float* buf, int framesPerBeat);

#endif // NOTE_DETECTION_H
