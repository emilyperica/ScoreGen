#ifndef MUSICXML_GENERATOR_H
#define MUSICXML_GENERATOR_H

#include <iostream>
#include <vector>
#include <string>
#include "libmusicxml.h"
#include "common.h"

using namespace std;
using namespace MusicXML2;

class MusicXMLGenerator {
    public:
        MusicXMLGenerator(
            const std::string& workNumber = "WORN_NUMBER",
            const std::string& workTitle = "WORK_TITLE",
            const std::string& movementNumber = "MVMT_NUMBER",
            const std::string& movementTitle = "MVMNT_TITLE",
            const std::string& creatorName = "CREATOR_NAME",
            const std::string& instrument = "INSTRUMENT",
            const std::string& timeSignature = "4/4"
        );
    
        ~MusicXMLGenerator();
    
        // Generates a MusicXML file at the specified output path.
        bool generate(
            const std::string& outputPath,
            const std::vector<XMLNote>& noteSequence,
            const std::string& clef,
            const int& clefLine,
            const int& keySignature,
            int divisions);
    
    private:
        TFactory factory;
        std::string instrument_;
        std::string timeSignature_;
    
        // Create a score-part element for the part-list.
        TElement createScorePart(
            const std::string& partId = "P1",
            const std::string& partName = "INSTRUMENT",
            const std::string& partAbbrev = "");  // Empty abbreviation to avoid extra output
    
        // Groups the note sequence into measures and creates the part element.
        TElement createPart(
            const std::vector<XMLNote>& noteSequence,
            const std::string& clef,
            const int& clefLine,
            const std::string& timeSignature,
            const int& keySignature,
            int divisions);
    
        // Creates a measure element from a collection of notes.
        TElement createMeasure(
            const std::vector<XMLNote>& measureNotes,
            int measureNumber,
            const std::string& clef,
            const int& clefLine,
            const std::string& timeSignature,
            const int& keySignature,
            int divisions);
    
        // Helper function to finish a measure: create a measure element from note elements and add it to the part.
        void finishMeasure(
            TElement part,
            std::vector<TElement>& noteElements,
            int measureNumber,
            const std::string& clef,
            int clefLine,
            const std::string& timeSignature,
            int keySignature,
            int divisions);
    
        // Creates a note element. If note.isRest is true, uses factoryRest; otherwise, factoryNote.
        // The durationOverride parameter allows specifying a partial duration.
        TElement createNoteElement(
            const XMLNote& note,
            int durationOverride,
            int divisions);
    
        // Helper to determine note type string based on duration and divisions.
        std::string getNoteTypeFromDuration(int duration, int divisions);

        friend class MusicXMLGeneratorTest;
    };

    
    #endif // MUSICXML_GENERATOR_H