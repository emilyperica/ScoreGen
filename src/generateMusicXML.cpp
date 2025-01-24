#include <fstream>
#include <vector>
#include <string>
#include "libmusicxml.h"
#include "generateMusicXML.h"

using namespace std;
using namespace MusicXML2;

// Initialize factory and set basic document info
MusicXMLGenerator::MusicXMLGenerator(
    const string& workNumber,
    const string& workTitle,
    const string& movementNumber,
    const string& movementTitle,
    const string& creatorName,
    const string& creatorType,
    const string& rightsString,
    const string& rightsType,
    const string& encodingSoftware)
{
    factory = factoryOpen();
    factoryHeader(factory, workNumber.c_str(), workTitle.c_str(), movementNumber.c_str(), movementTitle.c_str());
    factoryCreator(factory, creatorName.c_str(), creatorType.c_str());
    factoryRights(factory, rightsString.c_str(),rightsType.c_str());
    factoryEncoding(factory,encodingSoftware.c_str());
};

// Close factory and release memory, generation complete
MusicXMLGenerator::~MusicXMLGenerator() 
{
    factoryClose(factory);
};

// Generate MusicXML file from note sequence and rhythm (divisions) determined by our DSP
bool MusicXMLGenerator::generate(const vector<Note>& noteSequence, const string& outputPath,int divisions) {
    if (noteSequence.empty()) return false;

    TElement scorePart = createScorePart();
    factoryAddPart(factory, scorePart);
    TElement part = createPart(noteSequence, divisions);
    factoryAddPart(factory, part);

    // Print to file
    cout << "\nMusicXMLFactory writing to output file..." << endl;
    fstream outFile(outputPath, ios::out);
    if (!outFile.is_open()) return false;
    factoryPrint(factory, outFile);
    outFile.close();

    return true;
}

TElement MusicXMLGenerator::createScorePart(const string& partId, const string& partName, const string& partAbbrev)
{
   return factoryScorepart(factory, partId.c_str(), partName.c_str(), partAbbrev.c_str());
}

TElement MusicXMLGenerator::createPart(const vector<Note>& noteSequence, int divisions) 
{
    TElement part = factoryPart(factory, "P1");
    
    // Group notes into measures (assuming 4/4 time here)
    const int notesPerMeasure = 4 * divisions;
    int currentDivision = 0;
    int measureNumber = 1;
    vector<Note> currentMeasureNotes;

    for (const auto& note : noteSequence) 
    {
        currentMeasureNotes.push_back(note);
        currentDivision += note.duration;

        if (currentDivision >= notesPerMeasure) {
            TElement measure = createMeasure(currentMeasureNotes, measureNumber, divisions);
            factoryAddElement(factory, part, measure);
            currentMeasureNotes.clear();
            currentDivision = 0;
            measureNumber++;
        }
    }

    // Add last measure if not empty
    if (!currentMeasureNotes.empty()) 
    {
        TElement measure = createMeasure(currentMeasureNotes, measureNumber, divisions);
        factoryAddElement(factory, part, measure);
    }

    return part;
}

TElement MusicXMLGenerator::createMeasure(const vector<Note>& measureNotes, int measureNumber, int divisions) 
{
    // Create measure with attributes
    TElement measure = factoryMeasureWithAttributes(
        factory, 
        measureNumber, 
        "4/4", // Time signature
        "G", // Clef type
        2, // Clef line 
        0, // Key signature (0 = C major)
        divisions
    );

    // Add notes to measure
    vector<TElement> noteElements;
    for (const auto& note : measureNotes) 
    {
        TElement noteElement = createNoteElement(note, divisions);
        factoryAddElement(factory, measure, noteElement);
        noteElements.push_back(noteElement);
    }

    return measure;
}

TElement MusicXMLGenerator::createNoteElement(const Note& note, int divisions) 
{
    if (note.isRest){
        return factoryRest(factory, note.duration, note.type.c_str());
    }

    return factoryNote
    (
        factory, 
        note.pitch.c_str(), 
        note.alter,
        note.octave,
        note.duration,
        note.type.c_str()
    );
}