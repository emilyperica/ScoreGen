#include "determineBPM.h"


float calculateMedian(const std::vector<float>& values) {
    if (values.empty()) return 0.0;

    std::vector<float> sortedValues = values;
    std::sort(sortedValues.begin(), sortedValues.end());

    size_t n = sortedValues.size();
    if (n % 2 == 0) {
        return (sortedValues[n / 2 - 1] + sortedValues[n / 2]) / 2.0f;
    }
    else {
        return sortedValues[n / 2];
    }
}

// Helper function to calculate BPM
float beatsToBPM(const std::vector<float>& beats) {
    if (beats.size() > 1) {
        if (beats.size() < 4) {
            std::cout << "Few beats found." << std::endl;
        }
        std::vector<float> bpms;
        for (size_t i = 1; i < beats.size(); ++i) {
            bpms.push_back(60.0f / (beats[i] - beats[i - 1]));
        }
        return calculateMedian(bpms);
    }
    else {
        std::cout << "Not enough beats found." << std::endl;
        return 0.0f;
    }
}

// Function to calculate beats per minute (BPM) from a loaded buffer
float getBufferBPM(const std::vector<float>& buf, int sample_rate, const std::map<std::string, std::string>& params) {
    // Default settings
    int win_s = WIN_S, hop_s = HOP_S;

    // Handle different modes
    auto modeIt = params.find("mode");
    if (modeIt != params.end()) {
        const std::string& mode = modeIt->second;
        if (mode == "fast") {
            win_s = F_WIN_S; hop_s = F_HOP_S;
        }
        else if (mode == "super-fast") {
            win_s = SF_WIN_S; hop_s = SF_HOP_S;
        }
        else if (mode != "default") {
            throw std::invalid_argument("Unknown mode: " + mode);
        }
    }

    // Manual settings
    if (params.find("win_s") != params.end()) {
        win_s = std::stoi(params.at("win_s"));
    }
    if (params.find("hop_s") != params.end()) {
        hop_s = std::stoi(params.at("hop_s"));
    }

    // Initialize Aubio tempo detection
    aubio_tempo_t* tempo = new_aubio_tempo("specdiff", win_s, hop_s, sample_rate);
    if (!tempo) {
        throw std::runtime_error("Failed to initialize Aubio tempo detector");
    }

    std::vector<float> beats;
    fvec_t* input = new_fvec(hop_s);
    fvec_t* tempo_out = new_fvec(1); // Output buffer for tempo

    for (size_t i = 0; i < buf.size(); i += hop_s) {
        // Fill the input buffer
        for (size_t j = 0; j < hop_s && (i + j) < buf.size(); ++j) {
            fvec_set_sample(input, buf[i + j], j);
        }

        // Process the buffer to detect tempo
        aubio_tempo_do(tempo, input, tempo_out);
        if (fvec_get_sample(tempo_out, 0) != 0) { // Check for beat
            float this_beat = aubio_tempo_get_last_s(tempo);
            beats.push_back(this_beat);
        }
    }

    del_aubio_tempo(tempo);
    del_fvec(input);
    del_fvec(tempo_out);

    return beatsToBPM(beats);
}