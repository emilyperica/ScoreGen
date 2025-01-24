// Refs: 
    // Library GitHub repository: https://github.com/grame-cncm/libmusicxml.git
    // W3C MusicXML documentation: https://www.musicxml.com/
    // MusicXML DTD: https://www.musicxml.com/for-developers/musicxml-dtd/

#include <iostream>
#include <vector>
#include <string>
#include "libmusicxml.h"

#ifndef MUSICXML_GENERATOR_H
#define MUSICXML_GENERATOR_H

using namespace std;
using namespace MusicXML2;

struct Note 
{
    string pitch; // Note pitch (e.g., "C")
    float alter = 0; // Chromatic alteration 
    int octave = 4; // Octave
    int duration = 0; // Duration in divisions
    string type; // Note type (i.e. quarter, half, etc.)
    bool isRest = false; // Rest note flag
};

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

    bool generate(const vector<Note>& noteSequence, const string& outputPath, int divisions = 4); // Temp. assumed divisions

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