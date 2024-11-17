#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sndfile.h>
#include "detectNoteDuration.h"

using namespace std;

// hard-coded for now
#define BPM 60

/* Digital Signal Processing */
int dsp(const char* infilename) {
    SNDFILE* infile;
    SF_INFO sfinfo;

    // Initialize SF_INFO struct before use.
    memset(&sfinfo, 0, sizeof(sfinfo));

    // Open the input .wav file
    if (!(infile = sf_open(infilename, SFM_READ, &sfinfo))) {
        printf("Not able to open input file %s.\n", infilename);
        puts(sf_strerror(NULL));
        return 1;
    }

    // Calculate frames per second
    float framesPerBeat = (sfinfo.samplerate * sfinfo.channels) / (60.0 / BPM);
    float* buf = (float*)malloc(framesPerBeat * sizeof(float));

    if (!buf) {
        fprintf(stderr, "Memory allocation failed.\n");
        sf_close(infile);
        return 1;
    }

    // Call the note detection function
    detect_notes(infile, sfinfo, buf, framesPerBeat);

    // Clean up
    free(buf);
    sf_close(infile);

    return 0;
}
