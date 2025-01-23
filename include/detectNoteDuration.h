#ifndef NOTE_DETECTION_H
#define NOTE_DETECTION_H

#include <vector>
#include <string>
#include <map>

using namespace std;

// Struct to represent a detected note
struct Note {
    float startTime; // seconds
    float endTime;   // seconds
    string pitch;    // note name (e.g., "A4", "C#3")
    string type;     // note type (e.g., "quarter", "eighth")
};

// Function declarations
string getNoteName(double freq);
string determineNoteType(float noteDuration, int bpm);
vector<Note> detectNotes(const vector<float>& buf, int sample_rate, int channels, int bpm);

#endif
