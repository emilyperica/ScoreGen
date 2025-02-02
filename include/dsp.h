#ifndef DSP_H
#define DSP_H

#include <iostream>
#include <vector>
#include <sndfile.h>
#include "detectNoteDuration.h"
#include "common.h"
#include "findKey.h"

using namespace std;

struct DSPResult {
    vector<XMLNote> XMLNotes;
    string timeSignature;
    int keySignature;
    int bpm;
    int divisions;
};

DSPResult dsp(char const* input_file);

#endif
