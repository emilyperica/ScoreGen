#include <vector>
#include <iostream>
#include <sndfile.h>

using namespace std;

// Detection parameters
const float threshold = 0.01; // Amplitude threshold for detecting a note
const int silenceDurationFrames = 4410; // Silence duration to confirm note end (~0.1 seconds)

void detect_notes(SNDFILE* infile, SF_INFO& sfinfo, float* buf, int framesPerBeat) {
    bool foundFirstNote = false;
    double firstNoteTime = 0.0;
    double totalAudioLength = 0.0;
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
                        firstNoteTime = (double)(totalFrames - readCount + noteStartFrame) / (sfinfo.samplerate * sfinfo.channels);
                    }
                }
                else {
                    silenceCounter = 0; // Reset silence counter during a note
                }
            }
            else if (noteOngoing && amplitude < 0.005) {
                // Silence detected
                silenceCounter++;

                if (silenceCounter >= silenceDurationFrames) {
                    // Note ends
                    double noteStartTime = (double)(totalFrames - readCount + noteStartFrame) / (sfinfo.samplerate * sfinfo.channels);
                    double noteEndTime = (double)(totalFrames - readCount + i - silenceCounter) / (sfinfo.samplerate * sfinfo.channels);

                    notes.push_back({ noteStartTime, noteEndTime });

                    noteOngoing = false;
                    silenceCounter = 0;
                }
            }
        }

        // Handle ongoing note at the end of the buffer
        if (noteOngoing) {
            double noteStartTime = (double)(totalFrames - readCount + noteStartFrame) / (sfinfo.samplerate * sfinfo.channels);
            double noteEndTime = (double)totalFrames / (sfinfo.samplerate * sfinfo.channels);
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
                else if (noteDuration >= 0.35 && noteDuration < 0.4) {
                    cout << "Note is a Dotted Sixteenth Note" << endl;
                }
                else if (noteDuration >= 0.4 && noteDuration < 0.65) {
                    cout << "Note is an Eighth Note" << endl;
                }
                else if (noteDuration >= 0.65 && noteDuration < 1.2) {
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
}
