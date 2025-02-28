#ifndef NOTE_DETECTION_H
#define NOTE_DETECTION_H

#define _USE_MATH_DEFINES

#include <vector>
#include <string>
#include <cmath>
#include <map>
#include <iostream>
#include "aubio/aubio.h"
#include <fftw3.h>

#include "STFT.h"

// Struct to represent a detected note
struct Note {
    float startTime;
    float endTime;
    std::string pitch;
    std::string type;   // Note type (e.g., "quarter", "eighth")
};

// Function declarations
std::string getNoteName(double freq);
std::string determineNoteType(float noteDuration, int bpm);
std::vector<Note> detectNotes(const std::vector<double>& buf, int sample_rate, int channels);
std::string detectPitch(std::vector<std::vector<double>> spec, int startFrame, int endFrame, int sample_rate, int ffts);
std::vector<std::vector<double>> preProcessing(double lambda, std::vector<std::vector<double>> spectrogram);
std::vector<Note> onsetDetection(const std::vector<double>& buf, int sample_rate);

class Filter {
    public:
    int ffts;
    double fs;
    int bands;
    double fmin;
    double fmax;
    bool equal;
    std::vector<std::vector<double>> filterbank;

    Filter(int ffts, double fs, int bands = 12, double fmin = 27.5, double fmax = 16000, bool equal = false);
    std::vector<double> frequencies(int bands, double fmin, double fmax);
    static std::vector<double> triang(int start, int mid, int stop, bool equal = false);
};

#endif // NOTE_DETECTION_H
