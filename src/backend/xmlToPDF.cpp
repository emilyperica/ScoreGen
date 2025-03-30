#include "xmlToPDF.h"
#include "lilypond_paths.h"
#include <shlobj.h>
#include <filesystem>

bool fileExists(const std::string& filePath) {
    std::ifstream file(filePath);
    return file.good();
}

std::string getUniqueOutputPath(const std::string& baseName) {
    int counter = 1;
    std::string outputPath;
    std::string outputFile;

    TCHAR appdata[MAX_PATH] = {0};
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appdata);
    std::string outputDir = std::string(appdata) + "\\ScoreGen";
    if (_mkdir(outputDir.c_str()) == 0 || errno == EEXIST) {
    } else {
        std::cerr << "Error creating directory: " << outputDir << std::endl;
    }

    do {
        outputFile = baseName + "_" + std::to_string(counter);
        outputPath = outputDir + "\\" + outputFile;
        counter++;
    } while (fileExists(outputPath + ".pdf"));

    return outputFile;
}

void convertMusicXMLToPDF(const std::string& musicxmlPath, const std::string& outputPath) {
    std::string baseName = outputPath.substr(0, outputPath.find_last_of('.'));
    std::string uniqueFileName = getUniqueOutputPath(baseName);

    TCHAR appdata[MAX_PATH] = {0};
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appdata);
    std::string basePath = std::string(appdata) + "\\ScoreGen";

    std::string command1 = "\" \"" + LILYPOND_PYTHON + "\" \"" + MUSICXML2LY + "\" \"" 
                      + musicxmlPath + "\" -o \"" + baseName + ".ly\" \"";
    std::cout << command1 << std::endl;

    if (std::system(command1.c_str()) != 0) {
        std::cerr << "Error: musicxml2ly conversion failed\n";
        return;
    }
    std::string command2 = "\" \"" + LILYPOND_EXE + "\" --output=\"" + basePath 
                      + "\\" + uniqueFileName + "\" \"output.ly\" \"";
                      std::cout << command2 << std::endl;
    if (std::system(command2.c_str()) != 0) {
        std::cerr << "Error: LilyPond PDF generation failed\n";
        return;
    }
	std::remove("output.ly");

    std::cout << "PDF successfully generated: " << uniqueFileName << "\n";
}