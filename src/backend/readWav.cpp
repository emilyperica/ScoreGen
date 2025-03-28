#include <iostream>
#include <fstream>
#include <vector>

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