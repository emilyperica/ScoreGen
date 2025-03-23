#include <iostream>
#include <string>
#include "dsp.h"
#include "dsp2.h"
#include "generateMusicXML.h"
#include "recordAudio.h"
#include "postprocess.h"

#define DEFAULT_OUT "output.xml"
#define DEFAULT_TEST "test/TestingDatasets/Computer-Generated-Samples/D4_to_E5_1_second_per_note.wav"
#define DEFAULT_CLEF "G"
#define DEFAULT_CLEF_LINE 2
#define DEFAULT_TIME_SIG "4/4"
#define DEFAULT_DIVISIONS 480

void processAudio() {
    // DSPResult res = dsp("temp.wav");
    DSPResult res = dsp2("temp.wav");
    MusicXMLGenerator xmlGenerator;
    bool success = xmlGenerator.generate(
        DEFAULT_OUT, 
        res.XMLNotes, 
        DEFAULT_CLEF, 
        DEFAULT_CLEF_LINE, 
        DEFAULT_TIME_SIG, 
        res.keySignature, 
        DEFAULT_DIVISIONS
    );
    postProcessMusicXML(DEFAULT_OUT, DEFAULT_OUT);
    
    if (success) {
        // Print this exact line for Node to detect:
        std::cout << "MusicXML file generated successfully." << std::endl;
    } else {
        std::cout << "Failed to generate MusicXML file." << std::endl;
    }
}

int main() {
    std::string command;
    while (std::getline(std::cin, command)) {
        if (command == "processAudio") {
            processAudio();
        } else {
            std::cerr << "Unknown command: " << command << std::endl;
        }
    }
    return 0;
}
