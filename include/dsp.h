#ifndef DSP_H
#define DSP_H

#include <iostream>
#include <vector>
#include <sndfile.h>
#include "note_duration_extractor.h"
#include "determineBPM.h"
#include "common.h"
#include "findKey.h"

using namespace std;

DSPResult dsp(char const* input_file);

#endif
