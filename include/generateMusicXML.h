// Refs: 
// Library GitHub repository: https://github.com/grame-cncm/libmusicxml.git
// W3C MusicXML documentation: https://www.musicxml.com/
// MusicXML DTD: https://www.musicxml.com/for-developers/musicxml-dtd/

#include <iostream>
#include <vector>
#include <string>
#include "libmusicxml.h"
#include "common.h"

#ifndef MUSICXML_GENERATOR_H
#define MUSICXML_GENERATOR_H

using namespace MusicXML2;

#define KEY_SIG 0 // C major, +ve for sharps, -ve for flats
#define CLEF "G" // Clef in "G", "F", "C", "percussion", "TAB" or "none"
#define CLEF_LINE 2 // Treble clef line
#define TIME_SIG "4/4"
#define BEATS_PER_DIV 4 // Beats per division

class MusicXMLGenerator
{
public:
    // Note: some of the below parameters are optional and can be set to null or nullptr while others cannot
    // see libmusicxml.h
    MusicXMLGenerator(
        const std::string& workNumber = "WORN_NUMBER",
        const std::string& workTitle = "WORK_TITLE",
        const std::string& movementNumber = "MVMT_NUMBER",
        const std::string& movementTitle = "MVMNT_TITLE",
        const std::string& creatorName = "CREATOR_NAME",
        const std::string& creatorType = "CREATOR_TYPE",
        const std::string& rightsString = "RIGHTS_STRING",
        const std::string& rightsType = "RIGHTS_TYPE",
        const std::string& encodingSoftware = "ScoreGen");

    ~MusicXMLGenerator();

    bool generate(const std::vector<xmlNote>& noteSequence, const std::string& outputPath, int divisions);

private:
    TFactory factory;
    TElement createScorePart(
        const std::string& partId = "P1",
        const std::string& partName = "INSTRUMENT",
        const std::string& partAbbrev = "INST_ABBREV");
    TElement createPart(const std::vector<xmlNote>& noteSequence, int divisions);
    TElement createMeasure(const std::vector<xmlNote>& measureNotes, int measureNumber, int divisions);
    TElement createNoteElement(const xmlNote& note, int divisions);
};

#endif