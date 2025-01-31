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

using namespace std;
using namespace MusicXML2;

#define KEY_SIG 0 // C major, +ve for sharps, -ve for flats
#define CLEF "G" // Clef in "G", "F", "C", "percussion", "TAB" or "none"
#define CLEF_LINE 2 // Treble clef line
#define TIME_SIG "4/4"
#define BEATS_PER_DIV 4 // Beats per division


class MusicXMLGenerator 
{

public:
    // Note: some fof the below parameters are optional and can be set to null or nullptr while others cannot
    // see lihbmusicxml.h
    MusicXMLGenerator(
    const string& workNumber = "WORN_NUMBER",
    const string& workTitle = "WORK_TITLE",
    const string& movementNumber = "MVMT_NUMBER",
    const string& movementTitle = "MVMNT_TITLE",
    const string& creatorName = "CREATOR_NAME",
    const string& creatorType = "CREATOR_TYPE",
    const string& rightsString = "RIGHTS_STRING",
    const string& rightsType = "RIGHTS_TYPE",
    const string& encodingSoftware = "ScoreGen");

    ~MusicXMLGenerator();

    bool generate(const vector<Note>& noteSequence, const string& outputPath, int divisions);

private:
    TFactory factory;
    TElement createScorePart(
        const string& partId = "P1", 
        const string& partName = "INSTRUMENT", 
        const string& partAbbrev = "INST_ABBREV");
    TElement createPart(const vector<Note>& noteSequence, int divisions);
    TElement createMeasure(const vector<Note>& measureNotes, int measureNumber, int divisions);
    TElement createNoteElement(const Note& note, int divisions);
};

#endif