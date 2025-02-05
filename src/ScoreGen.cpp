#include <iostream>
#include "dsp.h"
#include "generateMusicXML.h"
#include "recordAudio.h"

#define TEST_DATA "test/TestingDatasets/Computer-Generated-Samples/D4_to_E5_1_second_per_note.wav"

#define DEFAULT_OUT "output.xml"
#define KEY_SIG 0 // C major, +ve for sharps, -ve for flats
#define CLEF "G" // Clef in "G", "F", "C", "percussion", "TAB" or "none"
#define CLEF_LINE 2 // Treble clef line
#define TIME_SIG "4/4"
#define DIVISIONS 480 // Divisions per beat

int main() {
    // record audio here
    recordAudio();
    // CLI app

    DSPResult res = dsp("recorded.wav");
    MusicXMLGenerator XMLgenerator;

    // Defaults are being used for clef, key, time, and divisions, see generateMusicXML.h
    bool succ = XMLgenerator.generate(DEFAULT_OUT, res.XMLNotes, CLEF, CLEF_LINE, TIME_SIG, res.keySignature, DIVISIONS);

    if (succ) {
        std::cout << "MusicXML file generated successfully." << std::endl;
    } else {
        std::cout << "Failed to generate MusicXML file." << std::endl;
    }
}