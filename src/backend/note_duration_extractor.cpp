#define _USE_MATH_DEFINES
#include "note_duration_extractor.h"

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
    
    double minFreq = 100.0;
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
    
    // Analysis parameters for pitch detection.
    int frameSize = 2048;  // larger window for robust pitch detection
    int hopSize   = 512;   // hop size in samples
    
    int totalSamples = static_cast<int>(audio.size());
    int numFrames = (totalSamples >= frameSize) ? ((totalSamples - frameSize) / hopSize + 1) : 0;
    
    std::vector<double> window = hanningFunction(frameSize);
    std::vector<double> pitchEstimates(numFrames, 0.0);
    std::vector<double> frameRMS(numFrames, 0.0); // RMS energy per pitch frame
    
    // Compute pitch estimates and RMS using the larger window.
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
    double tolerance = 0.05;         // allow ~4% pitch variation within a note
    double minNoteDuration = 60.0 / (bpm * 4); // minimum segment duration in seconds
    bool inSegment = false;
    bool isNoteSegment = false;      // true if current segment is a note, false if a rest
    double currentPitch = 0.0;       // used if in a note segment
    int segmentStartFrame = 0;
    std::vector<NoteSegment> segments;
    
    for (int i = 0; i < numFrames; i++) {
        double pitch = pitchEstimates[i];
        bool isRest = (pitch == 0);
        if (!inSegment) {
            inSegment = true;
            segmentStartFrame = i;
            isNoteSegment = !isRest;
            if (isNoteSegment)
                currentPitch = pitch;
        } else {
            if (isNoteSegment) {
                // End the note segment if a rest occurs or pitch deviates too much.
                if (isRest || std::abs(pitch - currentPitch) / currentPitch > tolerance) {
                    int segmentEndFrame = i - 1;
                    double startTime = segmentStartFrame * hopSize / static_cast<double>(sampleRate);
                    double endTime = (segmentEndFrame * hopSize + frameSize) / static_cast<double>(sampleRate);
                    double duration = endTime - startTime;
                    if (duration >= minNoteDuration) {
                        std::string noteName = frequencyToNoteString(currentPitch);
                        segments.push_back({noteName, segmentStartFrame, segmentEndFrame});
                    }
                    // Start a new segment.
                    inSegment = true;
                    segmentStartFrame = i;
                    isNoteSegment = !isRest;
                    if (isNoteSegment)
                        currentPitch = pitch;
                }
            } else {
                // In a rest segment, if a note is encountered, end the rest segment.
                if (!isRest) {
                    int segmentEndFrame = i - 1;
                    double startTime = segmentStartFrame * hopSize / static_cast<double>(sampleRate);
                    double endTime = (segmentEndFrame * hopSize + frameSize) / static_cast<double>(sampleRate);
                    double duration = endTime - startTime;
                    if (duration >= minNoteDuration) {
                        segments.push_back({"Rest", segmentStartFrame, segmentEndFrame});
                    }
                    inSegment = true;
                    segmentStartFrame = i;
                    isNoteSegment = true;
                    currentPitch = pitch;
                }
            }
        }
    }
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
    double mergeThreshold = 0.05; // seconds
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
    
    // --- Onset Detection with a Smaller Window ---
    int onsetFrameSize = 512;  // smaller window for onset detection
    int onsetHopSize = 256;    // higher time resolution
    int numOnsetFrames = (totalSamples >= onsetFrameSize) ?
                         ((totalSamples - onsetFrameSize) / onsetHopSize + 1) : 0;
    std::vector<double> onsetRMS(numOnsetFrames, 0.0);
    for (int i = 0; i < numOnsetFrames; i++) {
        int start = i * onsetHopSize;
        double sumSq = 0.0;
        for (int n = 0; n < onsetFrameSize; n++) {
            sumSq += audio[start + n] * audio[start + n];
        }
        onsetRMS[i] = std::sqrt(sumSq / onsetFrameSize);
    }
    // Collect onset times (in seconds) when the RMS difference exceeds a threshold.
    std::vector<double> onsetTimes;
    double onsetThresholdSmall = 0.02;  // adjust as needed
    for (int i = 1; i < numOnsetFrames; i++) {
        if ((onsetRMS[i] - onsetRMS[i - 1]) > onsetThresholdSmall) {
            double T = (i * onsetHopSize) / static_cast<double>(sampleRate);
            onsetTimes.push_back(T);
        }
    }
    
    // --- Split Note Segments at Onsets ---
    // For each merged note segment (ignoring rests), check if any onset (from the small-window analysis)
    // occurs within its time boundaries. If so, split the segment at the corresponding pitch-frame indices.
    std::vector<NoteSegment> finalSegments;
    for (const auto &seg : mergedSegments) {
        if (seg.note != "Rest") {
            double segStartTime = seg.startFrame * hopSize / static_cast<double>(sampleRate);
            double segEndTime = (seg.endFrame * hopSize + frameSize) / static_cast<double>(sampleRate);
            std::vector<int> splitPoints;
            // For each detected onset time, if it falls inside the segment, map it to a pitch-frame index.
            for (double T : onsetTimes) {
                if (T > segStartTime && T < segEndTime) {
                    int pitchFrameIndex = static_cast<int>(std::round(T * sampleRate / hopSize));
                    if (pitchFrameIndex > seg.startFrame && pitchFrameIndex <= seg.endFrame)
                        splitPoints.push_back(pitchFrameIndex);
                }
            }
            if (splitPoints.empty()) {
                finalSegments.push_back(seg);
            } else {
                int currentStart = seg.startFrame;
                for (int pf : splitPoints) {
                    int segmentEnd = pf - 1;
                    double startTime = currentStart * hopSize / static_cast<double>(sampleRate);
                    double endTime = (segmentEnd * hopSize + frameSize) / static_cast<double>(sampleRate);
                    double duration = endTime - startTime;
                    if (duration >= minNoteDuration)
                        finalSegments.push_back({seg.note, currentStart, segmentEnd});
                    currentStart = pf;
                }
                double startTimeFinal = currentStart * hopSize / static_cast<double>(sampleRate);
                double endTimeFinal = (seg.endFrame * hopSize + frameSize) / static_cast<double>(sampleRate);
                double durationFinal = endTimeFinal - startTimeFinal;
                if (durationFinal >= minNoteDuration)
                    finalSegments.push_back({seg.note, currentStart, seg.endFrame});
            }
        } else {
            finalSegments.push_back(seg);
        }
    }
    
    // Convert final segments to Note objects.
    for (const auto &seg : finalSegments) {
        double startTime = seg.startFrame * hopSize / static_cast<double>(sampleRate);
        double endTime = (seg.endFrame * hopSize + frameSize) / static_cast<double>(sampleRate);
        std::string noteType = determineNoteType((endTime - startTime), bpm);
        notes.push_back({static_cast<float>(startTime), static_cast<float>(endTime), seg.note, noteType});
        std::cout << "Note: " << seg.note << " | Start Time: " << startTime 
                  << " s | End Time: " << endTime << " s | Type: " << noteType << "\n";
    }
    return notes;
}
