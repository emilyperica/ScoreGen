#include "detectNoteDuration.h"

#define MAG_THRESHOLD 50
#define BPM 60  // default tempo
#define METER 4 // default num of beats per bar
#define BEAT 4  // default beat unit

// Pitch to frequency constants
#define A4 440
#define C0 (A4 * pow(2, -4.75))

// Detection parameters
const float threshold = 0.02f; // Amplitude threshold for detecting a note
const int silenceDurationFrames = 441; // Silence duration to confirm note end (~0.1 seconds)
const string note_names[12] = {"C", "C#", "D", "D#","E", "F", "F#", "G", "G#", "A", "A#", "B"};

// Beat units and corresponding float
const map<string, float> beats = {
    {"whole", 4.0f},
    {"half", 2.0f},
    {"quarter", 1.0f},
    {"eighth", 0.5f},
    {"sixteenth", 0.25f}
};

string getNoteName(double freq) {
    int h = round(12 * log2(freq / C0));
    int octave = h / 12;
    int n = h % 12;

    return note_names[n] + std::to_string(octave);
}

string detectPitch(std::vector<float> buf, size_t start, size_t end, int sample_rate) {
    int frames = end - start;
    vector<double> window(frames);
    string curr_note = "";
    double max_mag = 0.0;
    double dominant_freq = 0.0;

    // Perform FFT on time-domain data.
    fftw_complex *in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * frames);
    fftw_complex *out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * frames);
    fftw_plan plan = fftw_plan_dft_1d(frames, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // Apply Hamming window
    for (int i = 0; i < frames; i++) {
        window[i] = 0.54 - 0.46 * cos(2 * M_PI * i / (frames - 1));
        in[i][0] = buf[start + i] * window[i];
        in[i][1] = 0.0;
    }

    fftw_execute(plan);

    // Identify the dominant frequency present in the samples
    for (int i = 0; i < frames / 2; i++) {
        double magnitude = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        if (magnitude > MAG_THRESHOLD && magnitude > max_mag) {
            max_mag = magnitude;
            dominant_freq = i * sample_rate / (double)frames;;
        }
    }

    if (dominant_freq > 0.0) {
        curr_note = getNoteName(dominant_freq);
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    return curr_note;
}

// ----------------------- NOT IMPLEMENTED -----------------------
string determineNoteType(float noteDuration, int bpm, int beatsPerBar) {
    float beatDuration = 60.0f / BPM; // in seconds
    float beatsPerNote = noteDuration / beatDuration;
    float noteValue = beatsPerNote * BEAT; // note type in relation to the quarter note.

    return "";
}

vector<Note> detectNotes(const vector<float>& buf, int sample_rate, int channels) {
    int totalFrames = 0;
    size_t bufferSize = buf.size();
    vector<Note> notes;
    bool noteOngoing = false;
    int noteStartFrame = -1;
    int silenceCounter = 0;

    for (int i = 0; i < bufferSize; i++) {
        float amplitude = fabs(buf[i]);
        if (amplitude > threshold) {
            // Note starts
            if (!noteOngoing) {
                noteStartFrame = i;
                noteOngoing = true;
            }
            
            silenceCounter = 0; // Reset silence counter during a note
        } else if (noteOngoing && amplitude < 0.005) {
            // Silence detected
            silenceCounter++;
            if (silenceCounter >= silenceDurationFrames) {
                // Note ends
                float noteStartTime = (float)(noteStartFrame) / (sample_rate * channels);
                float noteEndTime = (float)(i - silenceCounter) / (sample_rate * channels);
                string pitch = detectPitch(buf, noteStartFrame, i, sample_rate);
                if (!pitch.empty()) {
                    string noteType = determineNoteType(noteEndTime - noteStartTime, BPM, METER);
                    notes.push_back({noteStartTime, noteEndTime, pitch, noteType});
                }
                
                noteOngoing = false;
                silenceCounter = 0;
            }
        }
    }

    return notes;
}
