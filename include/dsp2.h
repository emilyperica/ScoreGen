#ifndef DSP2_H
#define DSP2_H

#include <iostream>
#include <vector>
#include <sndfile.h>
#include "note_duration_extractor.h"
#include "common.h"
#include "findKey.h"

using namespace std;


DSPResult dsp2(char const* input_file);

#endif
