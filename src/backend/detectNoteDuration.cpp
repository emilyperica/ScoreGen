#include "detectNoteDuration.h"
#include "determineBPM.h"


FILE _iob[] = { *stdin, *stdout, *stderr };


using namespace std;

// Pitch to frequency constants
#define A4 440
#define C0 (A4 * pow(2, -4.75))

const string note_names[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

string getNoteName(double freq) {
    if (freq == 0) return "Rest";
    int h = round(12 * log2(freq / C0));
    int octave = h / 12;
    int n = h % 12;

    return note_names[n] + to_string(octave);
}
string determineNoteType(float noteDuration, int bpm) {
    // Predefined note durations
    map<string, float> note_durations = {
        {"sixteenth", 0.25},
        {"eighth", 0.5},
        {"quarter", 1.0},
        {"dotted quarter", 1.5},
        {"half", 2.0},
        {"dotted half", 3.0},
        {"whole", 4.0}
    };

    float beatDuration = 60.0f / bpm;
    float beatsPerNote = noteDuration / beatDuration;

    auto closest = min_element(
        note_durations.begin(),
        note_durations.end(),
        [beatsPerNote](const auto& a, const auto& b) {
            return abs(a.second - beatsPerNote) < abs(b.second - beatsPerNote);
        }
    );

    return closest->first; // Return the note name
}

vector<Note> detectNotes(const vector<float>& buf, int sample_rate, int channels) {
    vector<Note> notes;
    int bpm = getBufferBPM(buf, sample_rate);
    cout << "Calculated BPM: " << bpm << endl;

    // Initialize Aubio pitch detection
    aubio_pitch_t* pitch = new_aubio_pitch("default", 2048, 512, sample_rate);
    aubio_pitch_set_unit(pitch, "Hz");
    aubio_pitch_set_silence(pitch, -50); // Silence threshold in dB

    // Initialize Aubio onset detection
    aubio_onset_t* onset = new_aubio_onset("default", 2048, 512, sample_rate);

    // Buffers
    fvec_t* input = new_fvec(512);
    fvec_t* out_pitch = new_fvec(1);
    fvec_t* out_onset = new_fvec(1);

    bool noteOngoing = false;
    float noteStartTime = 0.0;
    float lastPitch = 0.0;
    int pitchCount = 0;

    for (size_t i = 0; i < buf.size(); i += 512) {
        // Fill the input buffer
        for (int j = 0; j < 512 && (i + j) < buf.size(); ++j) {
            fvec_set_sample(input, buf[i + j], j);
        }

        // Process pitch
        aubio_pitch_do(pitch, input, out_pitch);
        float detectedPitch = fvec_get_sample(out_pitch, 0);

        // Process onset
        aubio_onset_do(onset, input, out_onset);
        if (aubio_onset_get_last(onset) != 0 || i == 0) {
            // Onset detected (note start) or first iteration
            if (noteOngoing) {
                // Note end
                float noteEndTime = static_cast<float>(i) / (sample_rate * channels);
                string pitchName = getNoteName(lastPitch / pitchCount); // Use averaged pitch
                string noteType = "sixteenth";
                if (notes.empty()) {
                    notes.push_back({ noteStartTime, noteEndTime, pitchName, noteType });
                }
                else if (notes.back().pitch == pitchName) {
                    notes.back().endTime = noteEndTime;
                }
                else {
                    float duration = notes.back().endTime - notes.back().startTime;
                    notes.back().type = determineNoteType(duration, bpm);
                    notes.push_back({ noteStartTime, noteEndTime, pitchName, noteType });
                }
            }

            // Reset for the new note
            noteOngoing = true;
            noteStartTime = static_cast<float>(i) / (sample_rate * channels);
            lastPitch = detectedPitch;
            pitchCount = 1;
        }
        else if (noteOngoing) {
            // Continue counting the pitch for ongoing notes
            lastPitch += detectedPitch;
            pitchCount++;
        }
    }

    if (noteOngoing) {
        // Handle the last note if ongoing
        float noteEndTime = static_cast<float>(buf.size()) / (sample_rate * channels);
        string pitchName = getNoteName(lastPitch / pitchCount); // Use averaged pitch
        string noteType = determineNoteType(noteEndTime - notes.back().startTime, bpm);
        if (notes.empty()) {
            notes.push_back({ noteStartTime, noteEndTime, pitchName, noteType });
        }
        else if (notes.back().pitch == pitchName) {
            notes.back().endTime = noteEndTime;
            notes.back().type = noteType;
        }
        else {
            float duration = notes.back().endTime - notes.back().startTime;
            notes.back().type = determineNoteType(duration, bpm);
            notes.push_back({ noteStartTime, noteEndTime, pitchName, noteType });
        }
    }

    // Filter out notes with duration less than 0.03 seconds
    notes.erase(
        remove_if(notes.begin(), notes.end(), [](const Note& note) {
            return (note.endTime - note.startTime) < 0.03;
            }),
        notes.end()
    );

    // Cleanup
    del_aubio_pitch(pitch);
    del_aubio_onset(onset);
    del_fvec(input);
    del_fvec(out_pitch);
    del_fvec(out_onset);

    return notes;
}
