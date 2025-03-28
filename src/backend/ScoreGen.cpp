#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <shlobj.h>
#include <filesystem>
#include "dsp.h"
#include "generateMusicXML.h"
#include "recordAudio.h"
#include "postprocess.h"
#include "common.h"
#include "xmlToPDF.h"

#define DEFAULT_OUT "output.xml"
#define DEFAULT_TEST "test/TestingDatasets/Computer-Generated-Samples/D4_to_E5_1_second_per_note.wav"
#define DEFAULT_CLEF "G"
#define DEFAULT_CLEF_LINE 2
#define DEFAULT_TIME_SIG "4/4"
#define DEFAULT_DIVISIONS 480

bool has_valid_value(const std::unordered_map<std::string, std::string>& map, const std::string& key) {
    auto it = map.find(key);
    return it != map.end() && !it->second.empty();
}

std::map<std::string, std::string> parsePayload(const std::string& payload) {
    std::map<std::string, std::string> result;

    std::cout << "Received Payload: " << payload << std::endl;
    size_t start = payload.find('{');
    size_t end = payload.rfind('}');

    if (start == std::string::npos || end == std::string::npos || end <= start) {
        std::cout << "Warning: Invalid payload format" << std::endl;
        return result;
    }

    std::string body = payload.substr(start + 1, end - start - 1);
    std::istringstream iss(body);
    std::string token;
    while (std::getline(iss, token, ',')) {
        size_t colonPos = token.find(':');
        if (colonPos != std::string::npos) {
            std::string key = token.substr(0, colonPos);
            std::string value = token.substr(colonPos + 1);

            auto trim = [](std::string s) {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                    return !std::isspace(ch);
                    }));
                s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                    return !std::isspace(ch);
                    }).base(), s.end());

                if (!s.empty() && (s.front() == '\"' || s.front() == '\''))
                    s.erase(0, 1);
                if (!s.empty() && (s.back() == '\"' || s.back() == '\''))
                    s.pop_back();

                return s;
                };

            key = trim(key);
            value = trim(value);

            //std::cout << "Parsed: Key='" << key << "', Value='" << value << "'" << std::endl;

            result[key] = value;
        }
    }

    return result;
}

void processAudio(const std::map<std::string, std::string>& payload) {
    try {
        std::ifstream tempWav("temp.wav");
        if (!tempWav.good()) {
            std::cout << "Failed to generate MusicXML file. Temp WAV file not found." << std::endl;
            return;
        }

        TCHAR appdata[MAX_PATH] = {0};
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appdata);
    std::string fileName = std::string(appdata) + "\\ScoreGen\\temp.wav";

    DSPResult res = dsp(fileName.c_str());

        std::string workNumber = (payload.find("workNumber") != payload.end() && !payload.at("workNumber").empty()) ? payload.at("workNumber") : "Unnumbered Work";
        std::string workTitle = (payload.find("workTitle") != payload.end() && !payload.at("workTitle").empty()) ? payload.at("workTitle") : "Untitled Work";
        std::string movementNumber = (payload.find("movementNumber") != payload.end() && !payload.at("movementNumber").empty()) ? payload.at("movementNumber") : "Unnumbered Mvmt";
        std::string movementTitle = (payload.find("movementTitle") != payload.end() && !payload.at("movementTitle").empty()) ? payload.at("movementTitle") : "Untitled Mvmt";
        std::string creatorName = (payload.find("creatorName") != payload.end() && !payload.at("creatorName").empty()) ? payload.at("creatorName") : "Anon.";
        std::string instrument = (payload.find("instrumentInput") != payload.end() && !payload.at("instrumentInput").empty()) ? payload.at("instrumentInput") : "Piano";
        std::string timeSignature = (payload.find("timeSignatureInput") != payload.end() && !payload.at("timeSignatureInput").empty()) ? payload.at("timeSignatureInput") : DEFAULT_TIME_SIG;


        std::cout << "workNumber: " << workNumber << std::endl;
        std::cout << "workTitle: " << workTitle << std::endl;
        std::cout << "movementNumber: " << movementNumber << std::endl;
        std::cout << "movementTitle: " << movementTitle << std::endl;
        std::cout << "creatorName: " << creatorName << std::endl;
        std::cout << "instrument: " << instrument << std::endl;
        std::cout << "timeSignature: " << timeSignature << std::endl;

        MusicXMLGenerator xmlGenerator(workNumber, workTitle, movementNumber, movementTitle, creatorName, instrument, timeSignature);
        bool success = xmlGenerator.generate(
            DEFAULT_OUT,
            res.XMLNotes,
            DEFAULT_CLEF,
            DEFAULT_CLEF_LINE,
            res.keySignature,
            DEFAULT_DIVISIONS
        );

        if (success) {
            postProcessMusicXML(DEFAULT_OUT, DEFAULT_OUT);
            std::cout << "MusicXML file generated successfully." << std::endl;
        }
        else {
            std::cout << "Failed to generate MusicXML file." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cout << "Error in processAudio: " << e.what() << std::endl;
        std::cout << "Failed to generate MusicXML file." << std::endl;
    }
}

void generatePDF(const std::map<std::string, std::string>& payload) {
    processAudio(payload);
    convertMusicXMLToPDF(DEFAULT_OUT, "output.pdf");
}

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        std::string payloadStr;
        std::getline(iss, payloadStr);

        std::map<std::string, std::string> payload = parsePayload(payloadStr);

        if (command == "processAudio") {
            processAudio(payload);
        }
        else if (command == "generatePDF") {
            generatePDF(payload);
        }
        else {
            std::cerr << "Unknown command: " << command << std::endl;
        }
    }
    return 0;
}