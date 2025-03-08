#include <iostream>
#include <string>
#include "dsp.h"
#include "generateMusicXML.h"
#include "recordAudio.h"

// Default parameters
#define DEFAULT_TEST_DATA "test/TestingDatasets/Computer-Generated-Samples/D4_to_E5_1_second_per_note.wav"
#define DEFAULT_OUT "output.xml"
#define DEFAULT_KEY_SIG 0         // C major
#define DEFAULT_CLEF "G"          // e.g., "G", "F", "C", etc.
#define DEFAULT_CLEF_LINE 2       // For example, treble clef line
#define DEFAULT_TIME_SIG "4/4"
#define DEFAULT_DIVISIONS 480     // Divisions per beat

// This function encapsulates the audio processing functionality.
void processAudio() {
    //recordAudio();
    DSPResult res = dsp("recorded.wav");
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
    
    if (success) {
        std::cout << "MusicXML file generated successfully." << std::endl;
    } else {
        std::cout << "Failed to generate MusicXML file." << std::endl;
    }
}

int main() {
    std::string command;
    processAudio();
    
    // Continuously check for input on std::cin.
    // while (std::getline(std::cin, command)) {
    //     if (command == "processAudio") {
    //         processAudio();
    //     } else {
    //         std::cerr << "Unknown command: " << command << std::endl;
    //     }
    // }
    
    return 0;
}
