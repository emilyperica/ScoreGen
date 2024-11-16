#include <iostream>
#include <vector>
#include <cmath>

// Function to calculate duration in seconds
double calculateDuration(int startSample, int endSample, int sampleRate) {
    double duration = (endSample - startSample) / static_cast<double>(sampleRate);
std::cerr << "Duration (in seconds): " << duration << std::endl;
return duration;
}

// Function to map duration to note types
std::string mapToNoteType(double duration, double tempo) {
    
    double quarterNoteDuration = 60.0 / tempo; // Quarter note duration in seconds
    if (duration >= 4 * quarterNoteDuration) return "Whole Note";
    if (duration >= 2 * quarterNoteDuration) return "Half Note";
    if (duration >= quarterNoteDuration) return "Quarter Note";
    if (duration >= quarterNoteDuration / 2) return "Eighth Note";
    return "Smaller than Eighth Note";
}

// Core function: Process audio data directly from buffer
std::vector<std::pair<std::string, double>> detectNoteDuration(float* buf, int numFrames, int sampleRate, int channels, double tempo) {
    // Detect note onsets and offsets using amplitude threshold
    double threshold = 0.05; // Adjust threshold based on the recording
    std::vector<int> onsets;
    std::vector<int> offsets;
    bool inNote = false;

    for (size_t i = 0; i < numFrames; ++i) {
        // Use the first channel for amplitude analysis (simplified mono processing)
        double amplitude = std::abs(buf[i * channels]);

        if (amplitude > threshold) {
            if (!inNote) {
                inNote = true;
                onsets.push_back(i);  // Record the onset
            }
        }
        else {
            if (inNote) {
                inNote = false;
                offsets.push_back(i);  // Record the offset
            }
        }
    }

    // If there's one more onset than offset, handle it
    if (onsets.size() > offsets.size()) {
        std::cerr << "Mismatch detected: more onsets than offsets. Adjusting the last onset." << std::endl;
        onsets.pop_back();  // Remove last onset that doesn't have a corresponding offset
    }

    // Final mismatch check
    if (onsets.size() != offsets.size()) {
        std::cerr << "Mismatch in onsets and offsets detected. Exiting." << std::endl;
        std::cerr << "Onsets: " << onsets.size() << ", Offsets: " << offsets.size() << std::endl;
        exit(1);
    }

    // Calculate durations and map to note types
    std::vector<std::pair<std::string, double>> noteDurations;
    for (size_t i = 0; i < onsets.size(); ++i) {
        double duration = calculateDuration(onsets[i], offsets[i], sampleRate);
        std::string noteType = mapToNoteType(duration, tempo);
        noteDurations.emplace_back(noteType, duration);
    }

    return noteDurations;
}
