#include <gtest/gtest.h>
#include <fstream>
#include "common.h"
#include "generateMusicXML.h"

// Needed to expose private members of MusicXMLGenerator for testing, just wrappers
class MusicXMLGeneratorTest : public ::testing::Test {
protected:
    MusicXMLGenerator generator;

    MusicXMLGeneratorTest() : generator(
        "W001", 
        "Test Composition", 
        "M001", 
        "Test Movement", 
        "GTest", 
        "TestingSoftware"
    ) {}

    // Updated default parameters to match generate() logic
    TElement testCreateScorePart(
        const string& partId = "P1", 
        const string& partName = "INSTRUMENT", 
        const string& partAbbrev = ""
    ) {
        return generator.createScorePart(partId, partName, partAbbrev);
    }

    // Updated wrapper to include the duration override parameter
    TElement testCreateNoteElement(const XMLNote& note, int durationOverride, int divisions) {
        return generator.createNoteElement(note, durationOverride, divisions);
    }

    /*
    // Removed direct measure creation tests because measure creation is now handled
    // internally in createPart() and generate()
    TElement testCreateMeasure(
        const vector<XMLNote>& measureNotes, 
        int measureNumber,
        const string& clef = "G", 
        int clefLine = 2, 
        const string& timeSignature = "4/4", 
        int keySignature = 0, 
        int divisions = 4
    ) {
        return generator.createMeasure(
            measureNotes, 
            measureNumber, 
            clef, 
            clefLine, 
            timeSignature, 
            keySignature, 
            divisions
        );
    }
    */

    TElement testCreatePart(
        const vector<XMLNote>& noteSequence,
        const string& clef = "G", 
        int clefLine = 2, 
        const string& timeSignature = "4/4", 
        int keySignature = 0, 
        int divisions = 4
    ) {
        return generator.createPart(noteSequence, clef, clefLine, timeSignature, keySignature, divisions);
    }
};

TEST_F(MusicXMLGeneratorTest, NoteElementCreation) {
    // Quarter note at C, not flat/sharp, octave 4, duration 4 divisions, not a rest
    XMLNote regularNote = {"C", 0, 4, 4, "quarter", false};
    TElement regularNoteElement = testCreateNoteElement(regularNote, 4, 4);
    EXPECT_NE(regularNoteElement, nullptr);

    // Rest note with all fields
    XMLNote restNote = {"", 0, 0, 4, "quarter", true};
    TElement restNoteElement = testCreateNoteElement(restNote, 4, 4);
    EXPECT_NE(restNoteElement, nullptr);

    // Note with accidental (C-sharp)
    XMLNote accidentalNote = {"C", 1, 4, 4, "quarter", false};
    TElement accidentalNoteElement = testCreateNoteElement(accidentalNote, 4, 4);
    EXPECT_NE(accidentalNoteElement, nullptr);
}

/*
// Removed Measure creation tests because measures are created internally in createPart()
TEST_F(MusicXMLGeneratorTest, MeasureCreation) {
    // (Old tests for measure creation have been removed.)
}
*/

TEST_F(MusicXMLGeneratorTest, PartCreation) {
    // Multiple measures with various notes
    vector<XMLNote> noteSequence = {
        {"C", 0, 4, 4, "quarter", false},
        {"D", 0, 4, 4, "quarter", false},
        {"E", 0, 4, 4, "quarter", false},
        {"F", 0, 4, 4, "quarter", false},
        {"G", 1, 4, 4, "quarter", false},
        {"", 0, 0, 8, "half", true},
        {"A", 0, 4, 4, "quarter", false},
        {"B", -1, 4, 4, "quarter", false}
    };

    TElement part = testCreatePart(noteSequence);
    EXPECT_NE(part, nullptr);
}

TEST_F(MusicXMLGeneratorTest, ScorePartCreation) {
    TElement scorePart = testCreateScorePart();
    EXPECT_NE(scorePart, nullptr);
}

// TODO: Add tests for proper generation of all note types (e.g., dotted notes, rests, etc.)
TEST_F(MusicXMLGeneratorTest, AllNoteTypes) {}

TEST_F(MusicXMLGeneratorTest, FullGeneration) {
    vector<XMLNote> noteSequence = {
        // Measure 1: Standard notes and a double-sharp
        {"C", 0, 4, 4, "quarter", false},
        {"D", 1, 4, 4, "quarter", false},   // D-sharp
        {"E", 0, 4, 4, "quarter", false},
        {"F", 2, 4, 4, "quarter", false},     // F double-sharp

        // Measure 2: Double-flat and missing type for a non-rest note
        {"G", -2, 4, 4, "quarter", false},    // G double-flat
        {"A", 0, 4, 4, "", false},            // A note with missing type
        {"B", -1, 4, 4, "quarter", false},    // B-flat
        {"C", 0, 5, 4, "quarter", false},     // C in octave 5

        // Measure 3: Different durations and accidentals
        {"D", 0, 5, 8, "half", false},        // Half note
        {"E", -1, 5, 2, "eighth", false},     // E-flat eighth note
        {"F", 0, 5, 4, "quarter", false},
        {"G", 0, 5, 4, "quarter", false},

        // Measure 4: Rests with and without note type
        {"", 0, 0, 4, "quarter", true},       // Quarter rest with type specified
        {"", 0, 0, 8, "", true},              // Half rest without type
        {"A", 0, 5, 4, "quarter", false},     // Normal note after rests
        {"B", 0, 5, 4, "quarter", false},

        // Measure 5: Extreme octave values
        {"C", 0, 2, 4, "quarter", false},     // Lower octave note
        {"D", 1, 7, 4, "quarter", false}      // Higher octave with sharp
    };

    const char *filename = "full_generation_test.musicxml";
    
    EXPECT_TRUE(generator.generate(
        filename, 
        noteSequence, 
        "G", 
        2,
        0,
        4
    ));

    std::ifstream outputFile(filename);
    EXPECT_TRUE(outputFile.good());
    outputFile.seekg(0, std::ios::end);
    EXPECT_GT(outputFile.tellg(), 0);
    outputFile.close();
    std::remove(filename);

}

TEST_F(MusicXMLGeneratorTest, EmptySequenceGeneration) {
    vector<XMLNote> emptySequence;
    EXPECT_FALSE(generator.generate(
        "empty_output.xml", 
        emptySequence, 
        "G", 
        2, 
        0, 
        4
    ));
}
