#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <map>
#include "common.h"

// Helper function to read little-endian values.
template <typename T>
T readLE(std::ifstream &in) {
    T value;
    in.read(reinterpret_cast<char*>(&value), sizeof(T));
    return value;
}

//
// Function: readWav
// -----------------
// Reads a WAV file (16-, 24-, 32-bit PCM or 32-bit float)
// and mixes down multi-channel data into a mono signal with normalized
// samples in the range [-1, 1].
//
bool readWav(const std::string& filename, std::vector<double>& samples, int& sampleRate) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cerr << "Error: cannot open input file " << filename << "\n";
        return false;
    }

    // Read the RIFF header (first 12 bytes)
    char riff[4];
    in.read(riff, 4);
    if (std::strncmp(riff, "RIFF", 4) != 0) {
        std::cerr << "Error: not a valid RIFF file.\n";
        return false;
    }
    uint32_t chunkSize = readLE<uint32_t>(in);
    char wave[4];
    in.read(wave, 4);
    if (std::strncmp(wave, "WAVE", 4) != 0) {
        std::cerr << "Error: not a valid WAVE file.\n";
        return false;
    }

    // Read chunks until we find "fmt " chunk.
    char chunkID[4];
    uint32_t subchunkSize = 0;
    uint16_t audioFormat = 0;
    uint16_t numChannels = 0;
    uint32_t sRate = 0;
    uint32_t byteRate = 0;
    uint16_t blockAlign = 0;
    uint16_t bitsPerSample = 0;
    bool fmtFound = false;
    while (!fmtFound && in.read(chunkID, 4)) {
        subchunkSize = readLE<uint32_t>(in);
        if (std::strncmp(chunkID, "fmt ", 4) == 0) {
            fmtFound = true;
            audioFormat = readLE<uint16_t>(in);
            numChannels = readLE<uint16_t>(in);
            sRate = readLE<uint32_t>(in);
            byteRate = readLE<uint32_t>(in);
            blockAlign = readLE<uint16_t>(in);
            bitsPerSample = readLE<uint16_t>(in);
            if (subchunkSize > 16) {
                in.ignore(subchunkSize - 16);
            }
        } else {
            in.ignore(subchunkSize);
        }
    }
    if (!fmtFound) {
        std::cerr << "Error: fmt chunk not found.\n";
        return false;
    }
    sampleRate = sRate;

    // Search for the "data" chunk.
    bool dataFound = false;
    uint32_t dataSize = 0;
    while (!dataFound && in.read(chunkID, 4)) {
        dataSize = readLE<uint32_t>(in);
        if (std::strncmp(chunkID, "data", 4) == 0) {
            dataFound = true;
            break;
        } else {
            in.ignore(dataSize);
        }
    }
    if (!dataFound) {
        std::cerr << "Error: data chunk not found.\n";
        return false;
    }
    
    int bytesPerSample = bitsPerSample / 8;
    size_t totalFrames = dataSize / (numChannels * bytesPerSample);
    
    if (totalFrames > 100000000) {
        std::cerr << "Error: WAV file is too large to process as a single block." << std::endl;
        return false;
    }
    
    samples.resize(totalFrames);
    
    // Read samples and mix down to mono.
    for (size_t i = 0; i < totalFrames; i++) {
        double mixedSample = 0.0;
        for (int ch = 0; ch < numChannels; ch++) {
            double sampleValue = 0.0;
            if (audioFormat == 1) { // PCM integer
                if (bitsPerSample == 16) {
                    int16_t sampleInt;
                    in.read(reinterpret_cast<char*>(&sampleInt), sizeof(sampleInt));
                    sampleValue = sampleInt / 32768.0;
                } else if (bitsPerSample == 24) {
                    unsigned char bytes[3];
                    in.read(reinterpret_cast<char*>(bytes), 3);
                    int32_t sampleInt = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16);
                    if (sampleInt & 0x800000) sampleInt |= 0xFF000000;
                    sampleValue = sampleInt / 8388608.0;
                } else if (bitsPerSample == 32) {
                    int32_t sampleInt;
                    in.read(reinterpret_cast<char*>(&sampleInt), sizeof(sampleInt));
                    sampleValue = sampleInt / 2147483648.0;
                } else {
                    std::cerr << "Unsupported PCM bit depth: " << bitsPerSample << "\n";
                    return false;
                }
            } else if (audioFormat == 3) { // IEEE float
                if (bitsPerSample == 32) {
                    float sampleFloat;
                    in.read(reinterpret_cast<char*>(&sampleFloat), sizeof(sampleFloat));
                    sampleValue = sampleFloat;
                } else {
                    std::cerr << "Unsupported float bit depth: " << bitsPerSample << "\n";
                    return false;
                }
            } else {
                std::cerr << "Unsupported audio format: " << audioFormat << "\n";
                return false;
            }
            mixedSample += sampleValue;
        }
        samples[i] = mixedSample / numChannels;
    }
    
    return true;
}

//
// Function: hannWindow
// --------------------
// Returns a Hann window of length N.
//
std::vector<double> hannWindow(int N) {
    std::vector<double> w(N);
    for (int n = 0; n < N; n++) {
        w[n] = 0.5 * (1.0 - cos(2.0 * M_PI * n / (N - 1)));
    }
    return w;
}

//
// Function: detectPitch
// ---------------------
// Uses an autocorrelation–based method to estimate the dominant pitch (in Hz)
// from a windowed frame. Returns 0 if no clear pitch is detected.
//
double detectPitch(const std::vector<double>& frame, int sampleRate) {
    int N = frame.size();
    
    double r0 = 0.0;
    for (int i = 0; i < N; i++) {
        r0 += frame[i] * frame[i];
    }
    if (r0 < 1e-6)
        return 0.0;
    
    double minFreq = 50.0;
    double maxFreq = 2000.0;
    int maxLag = std::min(N - 1, static_cast<int>(sampleRate / minFreq));
    int minLag = std::max(1, static_cast<int>(sampleRate / maxFreq));
    
    double bestCorr = 0.0;
    int bestLag = 0;
    for (int lag = minLag; lag <= maxLag; lag++) {
        double sum = 0.0;
        for (int i = 0; i < N - lag; i++) {
            sum += frame[i] * frame[i + lag];
        }
        double normCorr = sum / r0;
        if (normCorr > bestCorr) {
            bestCorr = normCorr;
            bestLag = lag;
        }
    }
    if (bestCorr < 0.5)
        return 0.0;
    
    return sampleRate / static_cast<double>(bestLag);
}

//
// Function: frequencyToNoteString
// -------------------------------
// Converts a frequency (Hz) to a musical note name (e.g., "A4", "C#3").
// Uses A4 = 440 Hz as reference. Frequencies <= 0 return "Rest".
//
std::string frequencyToNoteString(double frequency) {
    if (frequency <= 0)
        return "Rest";
    int midiNote = static_cast<int>(std::round(12 * std::log2(frequency / 440.0))) + 69;
    const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F",
                               "F#", "G", "G#", "A", "A#", "B"};
    std::string note = noteNames[midiNote % 12];
    int octave = midiNote / 12 - 1;
    return note + std::to_string(octave);
}

// Internal structure to hold note segment information.
struct NoteSegment {
    std::string note;
    int startFrame; // First frame index of the note.
    int endFrame;   // Last frame index of the note.
};

std::string determineNoteType(float noteDuration, int bpm) {
    // Predefined note durations
    std::map<std::string, float> note_durations = {
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
    return closest->first;
}
//
// Function: extract_note_durations
// --------------------------------
// Processes the WAV file and returns a vector of Note objects that contain
// start time, end time, pitch (as a note string), and note type (set to "unknown").
// Now also performs a simple onset detection: if an onset is detected in the middle
// of a note segment, that segment is split into multiple notes.
//
std::vector<Note> extract_note_durations(const char* infilename, int bpm) {
    std::vector<Note> notes;
    std::vector<double> audio;
    int sampleRate;
    if (!readWav(infilename, audio, sampleRate)) {
        std::cerr << "Error reading WAV file.\n";
        return notes;
    }
    
    // Analysis parameters.
    int frameSize = 1024;  // frame length in samples
    int hopSize   = 512;   // hop size in samples
    
    int totalSamples = static_cast<int>(audio.size());
    int numFrames = (totalSamples >= frameSize) ? ((totalSamples - frameSize) / hopSize + 1) : 0;
    
    std::vector<double> window = hannWindow(frameSize);
    std::vector<double> pitchEstimates(numFrames, 0.0);
    std::vector<double> frameRMS(numFrames, 0.0); // store RMS energy per frame
    
    // Compute pitch estimates and RMS for each frame.
    for (int frame = 0; frame < numFrames; frame++) {
        int start = frame * hopSize;
        std::vector<double> frameBuffer(frameSize);
        double sumSq = 0.0;
        for (int n = 0; n < frameSize; n++) {
            frameBuffer[n] = audio[start + n] * window[n];
            sumSq += frameBuffer[n] * frameBuffer[n];
        }
        double rms = std::sqrt(sumSq / frameSize);
        frameRMS[frame] = rms;
        if (rms < 0.001) {
            pitchEstimates[frame] = 0.0;
        } else {
            double pitch = detectPitch(frameBuffer, sampleRate);
            pitchEstimates[frame] = pitch;
        }
    }
        
    // Segment frames into note and rest segments.
    double tolerance = 0.02;         // allow ~3% pitch variation within a note
    double minNoteDuration = 0.1;     // ignore segments shorter than 50 ms
    bool inSegment = false;
    bool isNoteSegment = false;      // true if current segment is a note, false if a rest
    double currentPitch = 0.0;       // only used if in a note segment
    int segmentStartFrame = 0;

    std::vector<NoteSegment> segments;
    for (int i = 0; i < numFrames; i++) {
        double pitch = pitchEstimates[i];
        bool isRest = (pitch == 0);

        if (!inSegment) {
            // Start a new segment.
            inSegment = true;
            segmentStartFrame = i;
            isNoteSegment = !isRest;
            if (isNoteSegment) {
                currentPitch = pitch;
            }
        } else {
            if (isNoteSegment) {
                // Currently in a note segment.
                // End the segment if we hit a rest or the pitch deviates too much.
                if (isRest || std::abs(pitch - currentPitch) / currentPitch > tolerance) {
                    int segmentEndFrame = i - 1;
                    double startTime = segmentStartFrame * hopSize / static_cast<double>(sampleRate);
                    double endTime = (segmentEndFrame * hopSize + frameSize) / static_cast<double>(sampleRate);
                    double duration = endTime - startTime;
                    if (duration >= minNoteDuration) {
                        std::string noteName = frequencyToNoteString(currentPitch);
                        segments.push_back({noteName, segmentStartFrame, segmentEndFrame});
                    }
                    // Start a new segment with the current frame.
                    inSegment = true;
                    segmentStartFrame = i;
                    isNoteSegment = !isRest;
                    if (isNoteSegment) {
                        currentPitch = pitch;
                    }
                }
                // Else, continue the current note segment.
            } else {
                // Currently in a rest segment.
                // If we encounter a non-rest (i.e. a note), end the rest segment.
                if (!isRest) {
                    int segmentEndFrame = i - 1;
                    double startTime = segmentStartFrame * hopSize / static_cast<double>(sampleRate);
                    double endTime = (segmentEndFrame * hopSize + frameSize) / static_cast<double>(sampleRate);
                    double duration = endTime - startTime;
                    if (duration >= minNoteDuration) {
                        segments.push_back({"Rest", segmentStartFrame, segmentEndFrame});
                    }
                    // Start a new note segment.
                    inSegment = true;
                    segmentStartFrame = i;
                    isNoteSegment = true;
                    currentPitch = pitch;
                }
                // Else, remain in the rest segment.
            }
        }
    }

    // End the last segment if still active.
    if (inSegment) {
        int segmentEndFrame = numFrames - 1;
        double startTime = segmentStartFrame * hopSize / static_cast<double>(sampleRate);
        double endTime = (segmentEndFrame * hopSize + frameSize) / static_cast<double>(sampleRate);
        double duration = endTime - startTime;
        if (duration >= minNoteDuration) {
            if (isNoteSegment) {
                std::string noteName = frequencyToNoteString(currentPitch);
                segments.push_back({noteName, segmentStartFrame, segmentEndFrame});
            } else {
                segments.push_back({"Rest", segmentStartFrame, segmentEndFrame});
            }
        }
    }

    // Merge adjacent segments with the same note if the gap is small.
    double mergeThreshold = 0.05; // seconds.
    std::vector<NoteSegment> mergedSegments;
    if (!segments.empty()) {
        mergedSegments.push_back(segments[0]);
        for (size_t i = 1; i < segments.size(); i++) {
            NoteSegment &prev = mergedSegments.back();
            NoteSegment &curr = segments[i];
            double prevEndTime = (prev.endFrame * hopSize + frameSize) / static_cast<double>(sampleRate);
            double currStartTime = (curr.startFrame * hopSize) / static_cast<double>(sampleRate);
            if (prev.note == curr.note && (currStartTime - prevEndTime) < mergeThreshold) {
                prev.endFrame = curr.endFrame;
            } else {
                mergedSegments.push_back(curr);
            }
        }
    }
    
    // --- Onset Detection ---
    // A simple energy-based onset detector: if the RMS energy increases by more than
    // a threshold between successive frames, mark that frame as an onset.
    std::vector<bool> onsetFlags(numFrames, false);
    double onsetThreshold = 0.03;  // adjust this threshold as needed
    for (int i = 1; i < numFrames; i++) {
        if ((frameRMS[i] - frameRMS[i - 1]) > onsetThreshold) {
            onsetFlags[i] = true;
        }
    }
    
    // --- Split Note Segments at Onsets ---
    // For each merged note segment (ignoring rests), check if an onset occurs within it.
    // If so, split the segment at each onset.
    std::vector<NoteSegment> finalSegments;
    for (const auto &seg : mergedSegments) {
        if (seg.note != "Rest") {
            std::vector<int> splitPoints;
            // Look for onsets strictly inside the segment (not at its very start)
            for (int i = seg.startFrame + 1; i <= seg.endFrame; i++) {
                if (onsetFlags[i]) {
                    splitPoints.push_back(i);
                }
            }
            if (splitPoints.empty()) {
                finalSegments.push_back(seg);
            } else {
                int currentStart = seg.startFrame;
                for (int onsetFrame : splitPoints) {
                    int segmentEnd = onsetFrame - 1;
                    double startTime = currentStart * hopSize / static_cast<double>(sampleRate);
                    double endTime = (segmentEnd * hopSize + frameSize) / static_cast<double>(sampleRate);
                    double duration = endTime - startTime;
                    if (duration >= minNoteDuration) {
                        finalSegments.push_back({seg.note, currentStart, segmentEnd});
                    }
                    currentStart = onsetFrame; // new segment starts at the onset
                }
                // Add the final segment from the last onset to seg.endFrame.
                double startTime = currentStart * hopSize / static_cast<double>(sampleRate);
                double endTime = (seg.endFrame * hopSize + frameSize) / static_cast<double>(sampleRate);
                double duration = endTime - startTime;
                if (duration >= minNoteDuration) {
                    finalSegments.push_back({seg.note, currentStart, seg.endFrame});
                }
            }
        } else {
            // Keep rest segments as they are.
            finalSegments.push_back(seg);
        }
    }
    
    // Convert final segments to Note objects.
    for (const auto &seg : finalSegments) {
        double startTime = seg.startFrame * hopSize / static_cast<double>(sampleRate);
        double endTime = (seg.endFrame * hopSize + frameSize) / static_cast<double>(sampleRate);
        std::string noteType = determineNoteType((endTime - startTime), bpm);
        // Here the note type is set to "unknown"; adjust if you implement rhythmic analysis.
        notes.push_back({static_cast<float>(startTime), static_cast<float>(endTime), seg.note, noteType});
        std::cout << "Note: " << seg.note << " " << endTime - startTime << " Type: " << noteType << "\n";
    }
    return notes;
}