#include "detectNoteDuration.h"
#include "processJson.h"


using namespace std;
#define REST_THRESHOLD 0.05


string midiToNoteName(int midiPitch) {
    // MIDI note names (index corresponds to midiPitch % 12)
    static const std::vector<std::string> noteNames = {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    // Compute note name and octave
    std::string note = noteNames[midiPitch % 12];
    int octave = (midiPitch / 12) - 1;

    return note + std::to_string(octave);
}

vector<Note> processNotes(const std::string& jsonPath, int bpm) {

    std::ifstream file(jsonPath);
    if (!file) {
        std::cerr << "Error: Could not open file " << jsonPath << std::endl;
        return {};
    }

    // Parse JSON
    nlohmann::json events;
    file >> events;
    file.close();

    std::vector<Note> notes;
    float latestTrebleEndTime = 0.0;
    float latestBassEndTime = 0.0;

    for (const auto& event : events) {
        if (event["confidence"] > 0.5) {  // Filter by confidence
            int midiPitch = event["midi_pitch"];
            std::string pitch = midiToNoteName(midiPitch);
            Staff staff = midiPitch < 59 ? Staff::Bass : Staff::Treble;
            float startTime = event["start_time"];
            float endTime = event["end_time"];
            float duration = endTime - startTime;
            std::string noteType = determineNoteType(duration, bpm);

            float& latestEndTime = (staff == Staff::Bass) ? latestBassEndTime : latestTrebleEndTime;

            // Insert a rest if the gap exceeds the threshold
            if (startTime - latestEndTime > REST_THRESHOLD) {
                float restDuration = startTime - latestEndTime;
                std::string restType = determineNoteType(restDuration, bpm);
                notes.push_back({
                    latestEndTime,    // Rest start time
                    startTime,        // Rest end time
                    "Rest",           // Indicate a rest
                    restType,
                    staff
                    });
            }

            // Add the actual note
            notes.push_back({
                startTime,
                endTime,
                pitch,
                noteType,
                staff
                });

            // Update latest note end time
            latestEndTime = endTime;
        }
    }

    return notes;
}