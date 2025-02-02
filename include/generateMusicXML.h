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

class MusicXMLGenerator 
{

public:
    // Note: some of the below parameters are optional and can be set to null or nullptr while others cannot
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

    bool generate(
        const string& outputPath,
        const vector<XMLNote>& noteSequence,
        const string& clef,
        const int& clefLine,
        const string& timeSignature,
        const int& keySignature,
        int divisions);

private:
    TFactory factory;
    TElement createScorePart(
        const string& partId = "P1", 
        const string& partName = "INSTRUMENT", 
        const string& partAbbrev = "INST_ABBREV");

    TElement createPart(const vector<XMLNote>& noteSequence,  const string& clef, 
        const int& clefLine, const string& timeSignature, const int& keySignature, int divisions);

    TElement createMeasure(const vector<XMLNote>& measureNotes, int measureNumber, const string& clef, 
        const int& clefLine, const string& timeSignature, const int& keySignature, int divisions);

    TElement createNoteElement(const XMLNote& note, int divisions);
};

#endif