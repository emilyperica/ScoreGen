// note_duration_extractor.cpp
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
#include <fftw3.h>
#include "common.h"

using namespace std;


#define A4 440
#define C0 (A4 * pow(2, -4.75))
const string note_names[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

// -------------------------
// WAV File Reading (mono conversion)
// -------------------------
template <typename T>
T readLE(ifstream &in) {
    T value;
    in.read(reinterpret_cast<char*>(&value), sizeof(T));
    return value;
}

bool readWav(const string& filename, vector<double>& samples, int& sampleRate) {
    ifstream in(filename, ios::binary);
    if (!in) {
        cerr << "Error: cannot open input file " << filename << "\n";
        return false;
    }
    char riff[4];
    in.read(riff, 4);
    if (strncmp(riff, "RIFF", 4) != 0) {
        cerr << "Error: not a valid RIFF file.\n";
        return false;
    }
    uint32_t chunkSize = readLE<uint32_t>(in);
    char wave[4];
    in.read(wave, 4);
    if (strncmp(wave, "WAVE", 4) != 0) {
        cerr << "Error: not a valid WAVE file.\n";
        return false;
    }
    char chunkID[4];
    uint32_t subchunkSize = 0;
    uint16_t audioFormat = 0, numChannels = 0;
    uint32_t sRate = 0, byteRate = 0;
    uint16_t blockAlign = 0, bitsPerSample = 0;
    bool fmtFound = false;
    while (!fmtFound && in.read(chunkID, 4)) {
        subchunkSize = readLE<uint32_t>(in);
        if (strncmp(chunkID, "fmt ", 4) == 0) {
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
        cerr << "Error: fmt chunk not found.\n";
        return false;
    }
    sampleRate = sRate;
    bool dataFound = false;
    uint32_t dataSize = 0;
    while (!dataFound && in.read(chunkID, 4)) {
        dataSize = readLE<uint32_t>(in);
        if (strncmp(chunkID, "data", 4) == 0) {
            dataFound = true;
            break;
        } else {
            in.ignore(dataSize);
        }
    }
    if (!dataFound) {
        cerr << "Error: data chunk not found.\n";
        return false;
    }
    
    int bytesPerSample = bitsPerSample / 8;
    size_t totalFrames = dataSize / (numChannels * bytesPerSample);
    samples.resize(totalFrames);
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
                    cerr << "Unsupported PCM bit depth: " << bitsPerSample << "\n";
                    return false;
                }
            } else if (audioFormat == 3) { // IEEE float
                if (bitsPerSample == 32) {
                    float sampleFloat;
                    in.read(reinterpret_cast<char*>(&sampleFloat), sizeof(sampleFloat));
                    sampleValue = sampleFloat;
                } else {
                    cerr << "Unsupported float bit depth: " << bitsPerSample << "\n";
                    return false;
                }
            } else {
                cerr << "Unsupported audio format: " << audioFormat << "\n";
                return false;
            }
            mixedSample += sampleValue;
        }
        samples[i] = mixedSample / numChannels;
    }
    return true;
}

// -------------------------
// Window Function & Pitch Detection
// -------------------------
vector<double> hannWindow(int N) {
    vector<double> w(N);
    for (int n = 0; n < N; n++) {
        w[n] = 0.5 * (1.0 - cos(2.0 * M_PI * n / (N - 1)));
    }
    return w;
}

// Autocorrelation-based pitch detection from a time-domain frame.
double detectPitch(const vector<double>& frame, int sampleRate) {
    int N = frame.size();
    double r0 = 0.0;
    for (int i = 0; i < N; i++) {
        r0 += frame[i] * frame[i];
    }
    if (r0 < 1e-4) // increased threshold to avoid false detections in silence
        return 0.0;
    
    double minFreq = 50.0;
    double maxFreq = 2000.0;
    int maxLag = min(N - 1, static_cast<int>(sampleRate / minFreq));
    int minLag = max(1, static_cast<int>(sampleRate / maxFreq));
    
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

// Converts a frequency (Hz) to a musical note name (e.g., "A4", "C#3").
// Frequencies <= 0 return "Rest".
string getNoteName(double freq) {
    if (freq <= 0) return "Rest";
    int h = round(12 * log2(freq / C0));
    int octave = h / 12;
    int n = h % 12;
    return note_names[n] + to_string(octave);
}

// Determine rhythmic note type from duration and BPM.
string determineNoteType(float noteDuration, int bpm) {
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
        note_durations.begin(), note_durations.end(),
        [beatsPerNote](const pair<string, float>& a, const pair<string, float>& b) {
            return fabs(a.second - beatsPerNote) < fabs(b.second - beatsPerNote);
        }
    );
    return closest->first;
}

// -------------------------
// FFTW-based STFT
// -------------------------
vector<vector<double>> computeSTFT(const vector<double>& audio, int fftSize, int hopSize) {
    int numFrames = (audio.size() - fftSize) / hopSize + 1;
    vector<vector<double>> spectrogram(numFrames, vector<double>(fftSize/2 + 1, 0.0));
    vector<double> window = hannWindow(fftSize);
    
    double* in = (double*) fftw_malloc(sizeof(double) * fftSize);
    fftw_complex* out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (fftSize/2 + 1));
    fftw_plan plan = fftw_plan_dft_r2c_1d(fftSize, in, out, FFTW_ESTIMATE);
    
    for (int frame = 0; frame < numFrames; frame++) {
        int start = frame * hopSize;
        for (int n = 0; n < fftSize; n++) {
            in[n] = audio[start + n] * window[n];
        }
        fftw_execute(plan);
        for (int k = 0; k <= fftSize/2; k++) {
            double re = out[k][0];
            double im = out[k][1];
            spectrogram[frame][k] = sqrt(re*re + im*im);
        }
    }
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
    return spectrogram;
}

// -------------------------
// Constant-Q Filter Bank for Onset Detection (from reference code)
// -------------------------
class Filter {
public:
    int ffts;
    double fs;
    int bands;
    double fmin;
    double fmax;
    bool equal;
    vector<vector<double>> filterbank;
    
    Filter(int ffts, double fs, int bands, double fmin, double fmax, bool equal)
        : ffts(ffts), fs(fs), bands(bands), fmin(fmin), fmax(fmax), equal(equal) {
        if (fmax > fs / 2) {
            fmax = fs / 2;
        }
        vector<double> frequencies = this->frequencies(bands, fmin, fmax);
        double factor = (fs / 2.0) / ffts;
        vector<int> freq_bins;
        for (double f : frequencies) {
            int bin = static_cast<int>(round(f / factor));
            if (bin < ffts) {
                freq_bins.push_back(bin);
            }
        }
        sort(freq_bins.begin(), freq_bins.end());
        freq_bins.erase(unique(freq_bins.begin(), freq_bins.end()), freq_bins.end());
        bands = freq_bins.size() - 2;
        filterbank.assign(ffts, vector<double>(bands, 0.0));
        for (int i = 0; i < bands; i++) {
            int start = freq_bins[i];
            int mid = freq_bins[i+1];
            int stop = freq_bins[i+2];
            vector<double> triangle = triang(start, mid, stop, equal);
            for (int j = start; j < stop; j++) {
                filterbank[j][i] = triangle[j - start];
            }
        }
    }
    
    vector<double> frequencies(int bands, double fmin, double fmax) {
        double factor = pow(2.0, 1.0 / bands);
        double freq = 440;
        vector<double> freqs = {freq};
        while (freq <= fmax) {
            freq *= factor;
            freqs.push_back(freq);
        }
        freq = 440;
        while (freq >= fmin) {
            freq /= factor;
            freqs.push_back(freq);
        }
        sort(freqs.begin(), freqs.end());
        return freqs;
    }
    
    vector<double> triang(int start, int mid, int stop, bool equal) {
        vector<double> triangle(stop - start, 0.0);
        double height = 1.0;
        if (equal) {
            height = 2.0 / (stop - start);
        }
        for (int i = 0; i < mid - start; i++) {
            triangle[i] = i * height / (mid - start);
        }
        for (int i = mid - start; i < triangle.size(); i++) {
            triangle[i] = height - (i - (mid - start)) * height / (stop - mid);
        }
        return triangle;
    }
};

// Preprocessing using a constant-Q filter bank and logarithmic scaling.
vector<vector<double>> preProcessing(double lambda, vector<vector<double>> spectrogram) {
    int ffts = 2048;
    double fs = 44100.0;
    int bands = 12;
    double fmin = 27.5;
    double fmax = 16000.0;
    bool equal = false;
    
    Filter filter(ffts, fs, bands, fmin, fmax, equal);
    vector<vector<double>> filterBank = filter.filterbank;
    
    int numTimeFrames = spectrogram.size();
    int numFreqBins = spectrogram[0].size();
    int numFilterBands = filterBank[0].size();
    vector<vector<double>> filteredSpec(numTimeFrames, vector<double>(numFilterBands, 0.0));
    for (int i = 0; i < numTimeFrames; i++) {
        for (int j = 0; j < numFilterBands; j++) {
            for (int k = 0; k < numFreqBins; k++) {
                filteredSpec[i][j] += spectrogram[i][k] * filterBank[k][j];
            }
        }
    }
    vector<vector<double>> logs(numTimeFrames, vector<double>(numFilterBands, 0.0));
    for (int i = 0; i < numTimeFrames; i++) {
        for (int j = 0; j < numFilterBands; j++) {
            logs[i][j] = log10(lambda * filteredSpec[i][j] + 1);
        }
    }
    return logs;
}

// -------------------------
// Onset Detection: Spectral Flux & Peak Picking
// -------------------------
vector<double> spectralFlux(vector<vector<double>> spectrogram) {
    vector<vector<double>> processedSpec = preProcessing(10, spectrogram);
    int numFrames = processedSpec.size();
    vector<double> diffs(numFrames, 0.0);
    for (int i = 1; i < numFrames; i++) {
        double flux = 0.0;
        int numBands = processedSpec[i].size();
        for (int j = 0; j < numBands; j++) {
            double diff = processedSpec[i][j] - processedSpec[i-1][j];
            if (diff > 0)
                flux += diff;
        }
        diffs[i] = flux;
    }
    return diffs;
}

// Peak picking with minimum separation (in frames)
vector<int> peakPicker(const vector<double>& flux, double fps, int minSeparation = 15) {
    vector<int> peaks;
    int N = flux.size();
    double maxFlux = *max_element(flux.begin(), flux.end());
    double delta = 0.5 * maxFlux; // adjustable threshold factor
    for (int i = 1; i < N - 1; i++) {
        if (flux[i] > flux[i-1] && flux[i] > flux[i+1] && flux[i] > delta) {
            if (!peaks.empty() && (i - peaks.back()) < minSeparation) {
                if (flux[i] > flux[peaks.back()])
                    peaks.back() = i;
            } else {
                peaks.push_back(i);
            }
        }
    }
    return peaks;
}

// Silence detection returns frame indices where energy is below threshold.
vector<int> silenceDetection(vector<vector<double>> spec, double silenceThreshold = 10, int minSilenceFrames = 9) {

    //TODO: my attempt at the silence detection, needs work
    int numFrames = spec.size();
    vector<int> silenceOnsets;
    vector<double> energies(numFrames, 0.0);
    for (int i = 0; i < numFrames; i++) {
        double sum = 0.0;
        for (double mag : spec[i]) {
            sum += mag;
        }
        energies[i] = sum;
    }
    int silentFrames = 0;
    for (int i = 0; i < numFrames; i++) {
        if (energies[i] < silenceThreshold) {
            silentFrames++;
            if (silentFrames >= minSilenceFrames) {
                silenceOnsets.push_back(i);
                silentFrames = 0;
            }
        } else {
            silentFrames = 0;
        }
    }
    return silenceOnsets;
}

// -------------------------
// Frequency-based Pitch Estimation from STFT Segments
// -------------------------
string detectPitch(const vector<vector<double>>& spec, int startFrame, int endFrame, int sample_rate, int ffts) {
    vector<vector<double>> noteSpec(spec.begin() + startFrame, spec.begin() + endFrame);
    double dominant_freq = 0.0;
    int numFrames = noteSpec.size();
    int numFreqBins = noteSpec[0].size();
    vector<double> avgSpectrum(numFreqBins, 0.0);
    for (const auto& frame : noteSpec) {
        for (int i = 0; i < numFreqBins; i++) {
            avgSpectrum[i] += frame[i] / numFrames;
        }
    }
    auto maxIter = max_element(avgSpectrum.begin(), avgSpectrum.end());
    int peakIndex = distance(avgSpectrum.begin(), maxIter);
    if (peakIndex <= 0 || peakIndex >= numFreqBins - 1) {
        dominant_freq = static_cast<double>(peakIndex) * sample_rate / ffts;
    } else {
        double alpha = avgSpectrum[peakIndex - 1];
        double beta = avgSpectrum[peakIndex];
        double gamma = avgSpectrum[peakIndex + 1];
        double den = alpha - 2 * beta + gamma;
        double correction = 0.5 * (alpha - gamma) / den;
        dominant_freq = (peakIndex + correction) * sample_rate / ffts;
    }
    // If frequency is too low, consider it silence.
    if (dominant_freq < 125)
        dominant_freq = 0;
    return getNoteName(dominant_freq);
}

std::vector<double> lowPassFilter(const std::vector<double>& input, double alpha) {
    std::vector<double> output(input.size(), 0.0);
    if (input.empty())
        return output;
    
    output[0] = input[0]; // initialize with first sample
    for (size_t i = 1; i < input.size(); i++) {
        output[i] = alpha * input[i] + (1.0 - alpha) * output[i - 1];
    }
    return output;
}

// -------------------------
// Hybrid Note Segmentation & Detection
// -------------------------
vector<Note> onsetDetection(const char* infilename, int sample_rate, int bpm) {
    int ffts = 2048;      // STFT window size
    double fs = sample_rate;
    int hopSize = 512;    // hop size in samples
    int fps = fs / hopSize;
    vector<double> buf;
    if (!readWav(infilename, buf, sample_rate)) {
        cerr << "Error reading WAV file.\n";
    }

    // Compute the spectrogram using FFTW-based STFT.
    vector<vector<double>> ft = computeSTFT(buf, ffts, hopSize);
    
    // Compute spectral flux.
    vector<double> flux = spectralFlux(ft);

    double alpha = 0.05;  // adjust alpha based on how much smoothing you need
    std::vector<double> smoothedFlux = lowPassFilter(flux, alpha);
    
    // Pick peaks (candidate onsets) using peakPicker.
    vector<int> peaks = peakPicker(smoothedFlux, fps, 7);
    
    // Incorporate silence onsets.
    vector<int> silOnsets = silenceDetection(ft, 10, 5);
    for (int s : silOnsets)
        peaks.push_back(s);
    sort(peaks.begin(), peaks.end());
    
    if (peaks.empty())
        return vector<Note>();
    
    // Use detected onset frames directly as segmentation boundaries.
    vector<Note> notes;
    for (size_t i = 0; i < peaks.size() - 1; i++) {
        int startFrame = peaks[i];
        int endFrame = peaks[i+1];
        double startTime = startFrame * hopSize / static_cast<double>(sample_rate);
        double endTime = (endFrame * hopSize + ffts) / static_cast<double>(sample_rate);
        if ((endTime - startTime) < 0.05) // ignore segments shorter than 50 ms -- this is causing issues with the rests and silence onset detection
            continue;
        string notePitch = detectPitch(ft, startFrame, endFrame, sample_rate, ffts);
        string noteType = determineNoteType(endTime - startTime, bpm);
        notes.push_back({static_cast<float>(startTime), static_cast<float>(endTime), notePitch, noteType});
    }

    return notes;
}
