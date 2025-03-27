#ifndef LILYPOND_CONVERTER_H
#define LILYPOND_CONVERTER_H

#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <direct.h>
#include "lilypond_paths.h" 

void convertMusicXMLToPDF(const std::string& musicxmlPath, const std::string& outputPath);

#endif
