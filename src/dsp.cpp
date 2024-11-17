#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sndfile.h>

using namespace std;


//hard-coded for now
#define BPM 60


/* Digital Signal Processing */
int dsp(char const* infilename) {
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

    cout << "Sample Rate: " << sfinfo.samplerate << endl;
    cout << "Channels: " << sfinfo.channels << endl;

    // Detection parameters
    const float threshold = 0.1; // Amplitude threshold for detecting a note
    const int silenceDurationFrames = sfinfo.samplerate / 10; // Silence duration to confirm note end (~0.1 seconds)

    bool foundFirstNote = false;
    double firstNoteTime = 0.0;
    double totalAudioLength = 0.0;

    int secondsProcessed = 0;
    int readCount;
    int totalFrames = 0;  // Keep track of the total number of frames read

    while ((readCount = sf_read_float(infile, buf, framesPerBeat)) > 0) {
        totalFrames += readCount;
        vector<pair<double, double>> notes; // Store start and end times for notes
        bool noteOngoing = false;
        int noteStartFrame = -1;

        int silenceCounter = 0;

        for (int i = 0; i < readCount; i++) {
            float amplitude = fabs(buf[i]);

            if (amplitude > threshold) {
                // Note starts
                if (!noteOngoing) {
                    noteStartFrame = i;
                    noteOngoing = true;
                    silenceCounter = 0;

                    // Record the first note start time if not already done
                    if (!foundFirstNote) {
                        foundFirstNote = true;
                        firstNoteTime = (totalFrames - readCount + noteStartFrame) / (double)sfinfo.samplerate;
                    }
                }
                else {
                    silenceCounter = 0; // Reset silence counter during a note
                }
            }
            else if (noteOngoing && amplitude < 0.008) {
                // Silence detected, check if it exceeds silence threshold
                silenceCounter++;

                if (silenceCounter >= silenceDurationFrames) {
                    // Note ends
                    double noteStartTime = (totalFrames - readCount + noteStartFrame) / (double)sfinfo.samplerate;
                    double noteEndTime = (totalFrames - readCount + i - silenceCounter) / (double)sfinfo.samplerate;

                    notes.push_back({ noteStartTime, noteEndTime });

                    noteOngoing = false;
                    silenceCounter = 0;
                }
            }
        }

        // Handle ongoing note at the end of the buffer
        if (noteOngoing) {
            double noteStartTime = (totalFrames - readCount + noteStartFrame) / (double)sfinfo.samplerate;
            double noteEndTime = totalFrames / (double)sfinfo.samplerate;
            notes.push_back({ noteStartTime, noteEndTime });
        }

        // Output detected notes for this chunk
        if (foundFirstNote) {
            for (const auto& note : notes) {
                cout << "Note Start: " << note.first << "s, End: " << note.second << "s" << endl;
                float noteDuration = note.second - note.first;

                if (noteDuration >= 0.2 && noteDuration < 0.35) {
                    cout << "Note is a Sixteenth Note" << endl;
                }
                else if (noteDuration >= 0.35 && noteDuration < 0.6) {
                    cout << "Note is a Dotted Sixteenth Note" << endl;
                }
                else if (noteDuration >= 0.6 && noteDuration < 0.8) {
                    cout << "Note is an Eighth Note" << endl;
                }
                else if (noteDuration >= 0.8 && noteDuration < 1.2) {
                    cout << "Note is a Quarter Note" << endl;
                }
                else if (noteDuration >= 1.2 && noteDuration < 1.6) {
                    cout << "Note is a Dotted Eighth Note" << endl;
                }
                else if (noteDuration >= 1.6 && noteDuration < 2.4) {
                    cout << "Note is a Half Note" << endl;
                }
                else if (noteDuration >= 2.4 && noteDuration < 3.2) {
                    cout << "Note is a Dotted Half Note" << endl;
                }
                else if (noteDuration >= 3.2 && noteDuration <= 4.8) {
                    cout << "Note is a Whole Note" << endl;
                }

            }
        }
    }


    // Calculate total length of the audio based on frames read
    totalAudioLength = (double)totalFrames / (sfinfo.samplerate * sfinfo.channels);

    // Output file details
    cout << "First note starts at: " << firstNoteTime << " seconds." << endl;
    cout << "Total audio length: " << totalAudioLength << " seconds." << endl;

    // Cleanup
    free(buf);
    sf_close(infile);

    return 0;
}
