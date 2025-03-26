#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "dsp.h"
#include "generateMusicXML.h"
#include "recordAudio.h"
#include "postprocess.h"

#define DEFAULT_OUT "output.xml"
#define DEFAULT_TEST "test/TestingDatasets/Computer-Generated-Samples/D4_to_E5_1_second_per_note.wav"
#define DEFAULT_CLEF "G"
#define DEFAULT_CLEF_LINE 2
#define DEFAULT_TIME_SIG "4/4"
#define DEFAULT_DIVISIONS 480


std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void processAudio(const std::string& workTitle, const std::string& workNumber,
    const std::string& movementNumber, const std::string& movementTitle,
    const std::string& creatorName, const std::string& instrument, const std::string& timeSignature) {

    DSPResult res = dsp("temp.wav");
    MusicXMLGenerator xmlGenerator(workNumber, workNumber, movementNumber, movementTitle, creatorName, instrument, timeSignature);
    bool success = xmlGenerator.generate(
        DEFAULT_OUT, 
        res.XMLNotes, 
        DEFAULT_CLEF, 
        DEFAULT_CLEF_LINE, 
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
    std::string input;
    while (std::getline(std::cin, input)) {
        if (input.find("processAudio") == 0) {
            std::vector<std::string> params = splitString(input, '|');

            if (params.size() == 8) {
                std::string command = params[0];
                std::string workTitle = params[1];
                std::string workNumber = params[2];
                std::string movementNumber = params[3];
                std::string movementTitle = params[4];
                std::string creatorName = params[5];
                std::string instrument = params[6];
                std::string timeSignature = params[7];

                processAudio(workTitle, workNumber, movementNumber, movementTitle, creatorName, instrument, timeSignature);
            }
            else {
                std::cerr << "Invalid number of parameters. Expected 8 but got " << params.size() << "." << std::endl;
            }
        }
        else {
            std::cerr << "Unknown command: " << input << std::endl;
        }
    }
    return 0;
}
