#include <fstream>
#include "generateMusicXML.h"

using namespace std;
using namespace MusicXML2;

//------------------------------------------------------------------------------
// Helper: Compute note type string from duration and divisions.
// For example, if divisions == 480:
//    480   -> "quarter"
//    960   -> "half"
//    1920  -> "whole"
//    240   -> "eighth"
// (Extend this mapping as needed.)
std::string MusicXMLGenerator::getNoteTypeFromDuration(int duration, int divisions) {
    if (duration == divisions)
        return "quarter";
    else if (duration == 2 * divisions)
        return "half";
    else if (duration == 4 * divisions)
        return "whole";
    else if (duration == divisions / 2)
        return "eighth";
    else if (duration == (3 * divisions) / 2)
        return "dotted-quarter";
    return "quarter";
}

//------------------------------------------------------------------------------
// Constructor: Initialize factory and set document info
//------------------------------------------------------------------------------
MusicXMLGenerator::MusicXMLGenerator(
    const string& workNumber,
    const string& workTitle,
    const string& movementNumber,
    const string& movementTitle,
    const string& creatorName,
    const string& instrument,
    const string& timeSignature)
    :
    instrument_(instrument),
    timeSignature_(timeSignature)
{
    factory = factoryOpen();
    factoryHeader(factory, workNumber.c_str(), workTitle.c_str(), movementNumber.c_str(), movementTitle.c_str());
    factoryCreator(factory, creatorName.c_str(), nullptr);
    factoryRights(factory, nullptr, nullptr);
    factoryEncoding(factory, "ScoreGen");
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
MusicXMLGenerator::~MusicXMLGenerator() 
{
    factoryClose(factory);
}

//------------------------------------------------------------------------------
// Generate the MusicXML file from the note sequence, grouping notes into measures
// and handling ties if a note crosses a barline.
//------------------------------------------------------------------------------
bool MusicXMLGenerator::generate(const string& outputPath,
                                 const vector<XMLNote>& noteSequence,
                                 const string& clef,
                                 const int& clefLine,
                                 const int& keySignature,
                                 int divisions)
{
    if (noteSequence.empty()) return false;

    TElement scorePart = createScorePart("P1", instrument_, "");
    factoryAddPart(factory, scorePart);

    TElement part = createPart(noteSequence, clef, clefLine, timeSignature_, keySignature, divisions);
    factoryAddPart(factory, part);

    fstream outFile(outputPath, ios::out);
    if (!outFile.is_open()) return false;
    factoryPrint(factory, outFile);
    outFile.close();

    return true;
}

//------------------------------------------------------------------------------
// Create a 'score-part' element with the given ID, name, and abbreviation.
//------------------------------------------------------------------------------
TElement MusicXMLGenerator::createScorePart(const string& partId,
                                            const string& partName,
                                            const string& partAbbrev)
{
    return factoryScorepart(factory, partId.c_str(), partName.c_str(), partAbbrev.c_str());
}

//------------------------------------------------------------------------------
// Create a part by grouping the note sequence into measures.
// This implementation splits notes crossing measure boundaries and ties them.
//------------------------------------------------------------------------------
TElement MusicXMLGenerator::createPart(const vector<XMLNote>& noteSequence,
                                       const string& clef,
                                       const int& clefLine,
                                       const string& timeSignature,
                                       const int& keySignature,
                                       int divisions)
{
    TElement part = factoryPart(factory, "P1");

    int beatsPerMeasure = stoi(timeSignature.substr(0, timeSignature.find('/')));
    int measureDivisions = beatsPerMeasure * divisions;
    
    int currentDivision = 0;
    int measureNumber = 1;
    vector<TElement> currentMeasureNotes;
    TElement tieCarryOver = nullptr;
    int tieDurationCarryOver = 0; // store the duration of the carried-over tied note

    for (size_t i = 0; i < noteSequence.size(); i++) {
        XMLNote note = noteSequence[i];
        int remainingNoteDuration = note.duration;
        
        // At the beginning of a measure, if there is a tied note carried over,
        // add it and update the currentDivision.
        if (currentDivision == 0 && tieCarryOver != nullptr) {
            currentMeasureNotes.push_back(tieCarryOver);
            currentDivision += tieDurationCarryOver;
            tieCarryOver = nullptr;
            tieDurationCarryOver = 0;
        }
        
        while (remainingNoteDuration > 0) {
            int spaceLeft = measureDivisions - currentDivision;
            if (spaceLeft <= 0) {
                finishMeasure(part, currentMeasureNotes, measureNumber, clef, clefLine, timeSignature, keySignature, divisions);
                measureNumber++;
                currentDivision = 0;
                currentMeasureNotes.clear();
                spaceLeft = measureDivisions;
                // If there's a carried-over tie note from before, add it.
                if (tieCarryOver != nullptr) {
                    currentMeasureNotes.push_back(tieCarryOver);
                    currentDivision += tieDurationCarryOver;
                    tieCarryOver = nullptr;
                    tieDurationCarryOver = 0;
                }
            }
            
            if (remainingNoteDuration <= spaceLeft) {
                XMLNote partialNote = note;
                partialNote.duration = remainingNoteDuration;
                partialNote.type = getNoteTypeFromDuration(partialNote.duration, divisions);
                TElement noteElem = createNoteElement(partialNote, partialNote.duration, divisions);
                currentMeasureNotes.push_back(noteElem);
                currentDivision += remainingNoteDuration;
                remainingNoteDuration = 0;
            } else {
                int durationThisMeasure = spaceLeft;
                int durationNext = remainingNoteDuration - durationThisMeasure;
                
                XMLNote partialNote1 = note;
                partialNote1.duration = durationThisMeasure;
                partialNote1.type = getNoteTypeFromDuration(durationThisMeasure, divisions);
                TElement noteElem1 = createNoteElement(partialNote1, partialNote1.duration, divisions);
                currentMeasureNotes.push_back(noteElem1);
                currentDivision += durationThisMeasure;
                remainingNoteDuration -= durationThisMeasure;
                
                finishMeasure(part, currentMeasureNotes, measureNumber, clef, clefLine, timeSignature, keySignature, divisions);
                measureNumber++;
                currentDivision = 0;
                currentMeasureNotes.clear();
                
                XMLNote partialNote2 = note;
                partialNote2.duration = durationNext;
                partialNote2.type = getNoteTypeFromDuration(durationNext, divisions);
                TElement noteElem2 = createNoteElement(partialNote2, partialNote2.duration, divisions);
                
                factoryTie(factory, noteElem1, noteElem2);
                
                // Set tieCarryOver and record its duration so it reduces space in the new measure.
                tieCarryOver = noteElem2;
                tieDurationCarryOver = durationNext;
                remainingNoteDuration = 0;
                break;
            }
        }
    }
    
    if (!currentMeasureNotes.empty()) {
        finishMeasure(part, currentMeasureNotes, measureNumber, clef, clefLine, timeSignature, keySignature, divisions);
    }
    
    return part;
}

//------------------------------------------------------------------------------
// finishMeasure: Helper function to create a measure element from note elements,
// then add it to the given part.
//------------------------------------------------------------------------------
void MusicXMLGenerator::finishMeasure(TElement part,
                                      vector<TElement>& noteElements,
                                      int measureNumber,
                                      const string& clef,
                                      int clefLine,
                                      const string& timeSignature,
                                      int keySignature,
                                      int divisions)
{
    TElement measureElem;
    if (measureNumber == 1) {
        measureElem = factoryMeasureWithAttributes(factory,
                                                   measureNumber,
                                                   timeSignature.c_str(),
                                                   clef.c_str(),
                                                   clefLine,
                                                   keySignature,
                                                   divisions);
    } else {
        measureElem = factoryMeasure(factory, measureNumber);
    }
    
    for (auto& elem : noteElements) {
        factoryAddElement(factory, measureElem, elem);
    }
    
    factoryAddElement(factory, part, measureElem);
}

//------------------------------------------------------------------------------
// createNoteElement: Creates a note element. If note.isRest is true, uses factoryRest;
// otherwise, factoryNote. The durationOverride is used to set the note's duration.
//------------------------------------------------------------------------------
TElement MusicXMLGenerator::createNoteElement(const XMLNote& note, int durationOverride, int divisions)
{
    int usedDuration = (durationOverride > 0) ? durationOverride : note.duration;
    std::string noteType = getNoteTypeFromDuration(usedDuration, divisions);
    
    if (note.isRest) {
        return factoryRest(factory, usedDuration, noteType.c_str());
    }
    
    return factoryNote(factory,
                       note.pitch.c_str(),
                       note.alter,
                       note.octave,
                       usedDuration,
                       noteType.c_str());
}
