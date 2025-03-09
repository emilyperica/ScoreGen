#include "detectNoteDuration.h"
#include "determineBPM.h"
#include "common.h"


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

// onset detection based on https://github.com/CPJKU/onset_detection/blob/master/onset_program.py and https://archives.ismir.net/ismir2012/paper/000049.pdf 
Filter::Filter(int ffts, double fs, int bands, double fmin, double fmax, bool equal)
        : ffts(ffts), fs(fs), bands(bands), fmin(fmin), fmax(fmax), equal(equal) 
{
    // Reduce fmax if necessary
    if (fmax > fs / 2) {
        fmax = fs / 2;
    }

    std::vector<double> frequencies = Filter::frequencies(bands, fmin, fmax);
    double factor = (fs / 2.0) / ffts;
    std::vector<int> freq_bins;
    for (double f : frequencies) {
        int bin = static_cast<int>(std::round(f / factor));
        if (bin < ffts) {
            freq_bins.push_back(bin);
        }
    }

    // remove duplicates
    std::sort(freq_bins.begin(), freq_bins.end());
    freq_bins.erase(std::unique(freq_bins.begin(), freq_bins.end()), freq_bins.end());

    bands = freq_bins.size() - 2;
    filterbank.assign(ffts, std::vector<double>(bands, 0.0));
    // create triangular filters
    for (int i = 0; i < bands; i++) {
        int start = freq_bins[i];
        int mid = freq_bins[i+1];
        int stop = freq_bins[i+2];

        std::vector<double> triangle = this->triang(start, mid, stop, equal);
        for (int j = start; j < stop; j++) {
            filterbank[j][i] = triangle[j-start];
        }
    }
}

std::vector<double> Filter::frequencies(int bands, double fmin, double fmax) {
    // factor 2 frequencies are apart
    double factor = pow(2.0, (1.0/bands));
    double freq = 440;
    std::vector<double> frequencies = {freq};

    // generate frequencies upwards from A0 to fmax
    while (freq <= fmax) {
        freq *= factor;
        frequencies.push_back(freq);
    }

    // generate frequencies downwards from A0 to fmin
    freq = 440;
    while (freq >= fmin) {
        freq /= factor;
        frequencies.push_back(freq);
    }

    std::sort(frequencies.begin(), frequencies.end());
    return frequencies;
}

std::vector<double> Filter::triang(int start, int mid, int stop, bool equal) {
    std::vector<double> triangle(stop - start, 0.0);
    double height = 1.0;

    // Normalize height if needed
    if (equal) {
        height = 2.0 / (stop - start);
    }

    // Rising edge
    for (int i = 0; i < mid-start; i++) {
        triangle[i] = i * height/(mid-start);
    }

    // Falling edge
    for (int i = mid-start; i < triangle.size(); i++) {
        triangle[i] = height - (i - (mid - start))*height/(stop - mid);
    }

    return triangle;
}

std::vector<std::vector<double>> preProcessing(double lambda, std::vector<std::vector<double>> spectrogram) { // lambda is chosen to be between 0.01 and 20
    // Constant-Q
    int ffts = 2048;
    double fs = 44100.0;
    int bands = 12;
    double fmin = 27.5;
    double fmax = 16000.0;
    bool equal = false;

    Filter filter(ffts, fs, bands, fmin, fmax, equal);
    std::vector<std::vector<double>> filterBank = filter.filterbank;

    int numTimeFrames = spectrogram.size();
    int numFreqBins = spectrogram[0].size();
    int numFilterBands = filterBank[0].size();
    std::vector<std::vector<double>> filteredSpec(numTimeFrames, std::vector<double>(numFilterBands, 0.0));

    // multiply the spectrogram by the filter bank
    for (int i = 0; i < numTimeFrames; i++) {
        for (int j = 0; j < numFilterBands; j++) {
            for (int k = 0; k < numFreqBins; k++) {
                filteredSpec[i][j] += spectrogram[i][k] * filterBank[k][j];
            }
        }
    }

    // logarithmic filter of spectrogram
    std::vector<std::vector<double>> logs(numTimeFrames, std::vector<double>(numFilterBands, 0.0));
    for (int i = 0; i < filteredSpec.size(); i++) {
        for (int j = 0; j < filteredSpec[i].size(); j++) {
            logs[i][j] = log10(lambda * filteredSpec[i][j] + 1);
        }
    }

    return logs;
}


// onset detection function
std::vector<double> spectralFlux(std::vector<std::vector<double>> spectrogram) {
    std::vector<std::vector<double>> processedSpec = preProcessing(10, spectrogram);
    std::vector<double> diffs(processedSpec.size(), 0.0);

    for (int i = 1; i < processedSpec.size(); i++) {
        double flux = 0.0;
        for (int j = 0; j < processedSpec[i].size(); j++) {
            // applying the half-wave rectifier function
            double x = abs(processedSpec[i][j]) - abs(processedSpec[i-1][j]);
            flux += (x+abs(x))/2;
        }

        diffs[i] = flux;
    }

    return diffs;
}

std::vector<double> peakPicker(std::vector<double> onsets, double fps) {
    
    std::vector<double> peaks;
    std::vector<double> means(onsets.size());
    std::vector<double> maximums(onsets.size());
    
    // parameter values (calibrate as necessary)
    double delta = 20; // must be determined empirically 
    //double delta = 0.2 * *std::max_element(onsets.begin(), onsets.end());
    int w1 = 3;
    int w2 = 3;
    int w3 = 8; // optimal value is between 4 and 12
    int w4 = 1;
    int w5 = static_cast<int>(30.0 / 1000.0 * fps);

    // step 1: get the list of means and maximums
    double movingMean = 0;
    double movingMax = 0;
    int front = 0;
    int end = 0;
    int meansWinSize = w3 + w4 + 1;
    for (int i=0; i < w4; i++) {
        movingMean += onsets[i];
    }

    movingMean /= w4;
    
    for (int n = 0; n < onsets.size(); n++) {
        // calculating means of each window
        if (n - w3 <= 0) {
            movingMean = (movingMean * (n+w4) + onsets[n+w4]) / (n+w4+1);
        } else if (n + w4 >= onsets.size()) {
            meansWinSize = onsets.size() - n + w3;
            movingMean = (movingMean * (meansWinSize + 1) - onsets[n-w3-1]) / meansWinSize;
        } else {
            movingMean = (movingMean * (meansWinSize) - onsets[n-w3-1] + onsets[n+w4]) / meansWinSize;
        }

        // calculate maximums of each window
        if (n - w1 < 0) {
            front = 0;
        } else if (n + w2 >= onsets.size()) {
            end = onsets.size() - 1;
        } else {
            front = n - w1;
            end = n + w2;
        }

        movingMax = 0;
        for (int i = front; i <= end; i++) {
            if (onsets[i] > movingMax) {
                movingMax = onsets[i];
            }
        }

        means[n] = movingMean;
        maximums[n] = movingMax;
    }

    // step 2: get the peaks
    int lastOnsetIndex = -1;
    for (int i=0; i < onsets.size(); i++) {
        if ((onsets[i] >= means[i] + delta) && (onsets[i] == maximums[i]) && ((i-lastOnsetIndex > w5) || lastOnsetIndex < 0)) {
            peaks.push_back(i);
            lastOnsetIndex = i;
        }
    }

    return peaks;
}


std::vector<Note> onsetDetection(const std::vector<double>& buf, int sample_rate, int bpm) {
    int ffts = 2048;           // window size for STFT
    double fs = sample_rate;
    int hopSize = 441;         // hop size in samples
    int fps = fs / hopSize;    // frames per second for STFT output

    // Compute the spectrogram using your STFT function.
    std::vector<std::vector<double>> ft = STFT(buf, ffts, hopSize);

    // Compute spectral flux and pick peaks as candidate note onsets.
    std::vector<double> onsets = spectralFlux(ft);
    std::vector<double> peaks = peakPicker(onsets, fps);

    // Incorporate additional silent onsets (e.g., from silenceDetection).
    std::vector<double> silentOnsets = silenceDetection(ft, peaks, fps);
    if (!silentOnsets.empty() && silentOnsets[0] < peaks[0]) {
        peaks[0] = silentOnsets[0];
    }
    peaks.insert(peaks.end(), silentOnsets.begin(), silentOnsets.end());
    std::sort(peaks.begin(), peaks.end());

    if (peaks.empty()) {
        return std::vector<Note>();
    }

    // Form initial note segments based on consecutive peaks.
    std::vector<Note> notes;
    for (size_t i = 0; i < peaks.size() - 1; i++) {
        Note note;
        double rawStart = peaks[i] / static_cast<double>(fps);
        double rawEnd   = peaks[i + 1] / static_cast<double>(fps);
        // For the very first note, force the start time to 0 (i.e. ignore the prepended silence)
        if (i == 0) {
            note.startTime = 0.0;
            note.endTime   = rawEnd;
        } else {
            note.startTime = rawStart;
            note.endTime   = rawEnd;
        }
        note.pitch = detectPitch(ft, peaks[i], peaks[i + 1], fs, ffts);
        note.type = determineNoteType(note.endTime - note.startTime, bpm);
        notes.push_back(note);
    }
    // Handle the final note segment.
    Note lastNote;
    double rawLastStart = peaks.back() / static_cast<double>(fps);
    double rawLastEnd = static_cast<double>(onsets.size() - 1) / static_cast<double>(fps);
    // For the final note, if no previous note exists, force its start to 0;
    // otherwise, leave it unchanged.
    if (notes.empty()) {
        lastNote.startTime = 0.0;
        lastNote.endTime = rawLastEnd;
    } else {
        lastNote.startTime = rawLastStart;
        lastNote.endTime = rawLastEnd;
    }
    lastNote.pitch = detectPitch(ft, peaks.back(), ft.size(), fs, ffts);
    lastNote.type = determineNoteType(lastNote.endTime - lastNote.startTime, bpm);
    notes.push_back(lastNote);

    // Merge adjacent note segments if they have the same pitch and the gap between them is small.
    double mergeThreshold = 0.1; // merge if gap is less than 0.1 seconds
    std::vector<Note> mergedNotes;
    mergedNotes.push_back(notes[0]);
    for (size_t i = 1; i < notes.size(); i++) {
        Note& prev = mergedNotes.back();
        if (notes[i].pitch == prev.pitch && (notes[i].startTime - prev.endTime) < mergeThreshold) {
            prev.endTime = notes[i].endTime;
            prev.type = determineNoteType(prev.endTime - prev.startTime, bpm);
        } else {
            mergedNotes.push_back(notes[i]);
        }
    }
    //for debugging
    // for (const Note& note : mergedNotes) {
    //     std::cout << note.startTime << " - " << note.endTime 
    //               << "\t" << note.type << "\t" << note.pitch << "\n";
    // }

    return mergedNotes;
}


string detectPitch(std::vector<std::vector<double>> spec, int startFrame, int endFrame, int sample_rate, int ffts) {
    std::vector<std::vector<double>> noteSpec(spec.begin() + startFrame, spec.begin() + endFrame);
    double dominant_freq = 0.0;
    int restThreshold = 120; // empirically determined

    int numFrames = noteSpec.size();
    int numFreqBins = noteSpec[0].size();

    std::vector<double> avgSpectrum(numFreqBins, 0.0);
    for (const auto& frame : noteSpec) {
        for (int i = 0; i < numFreqBins; i++) {
            avgSpectrum[i] += frame[i] / numFrames;
        }
    }

    auto maxIter = std::max_element(avgSpectrum.begin(), avgSpectrum.end());
    int peakIndex = std::distance(avgSpectrum.begin(), maxIter);
    if (peakIndex <= 0 || peakIndex >= avgSpectrum.size() - 1) {
        dominant_freq = static_cast<double>(peakIndex) * sample_rate / ffts;
    } else {
         // perform peak interpolation
        double alpha = avgSpectrum[peakIndex - 1];
        double beta = avgSpectrum[peakIndex];
        double gamma = avgSpectrum[peakIndex + 1];
        double den = alpha - 2*beta + gamma;
        double correction = 0.5 * (alpha - gamma) / den;
        
        dominant_freq = static_cast<double>(peakIndex + correction) * sample_rate / ffts;
    }
    
    dominant_freq *= (dominant_freq > 125);

    return getNoteName(dominant_freq);
}

std::vector<double> silenceDetection(std::vector<std::vector<double>> spec, std::vector<double> peaks, double fps) {
    double silenceThreshold = 10;
    int minSilenceFrames = 5;
    std::vector<double> energies;
    std::vector<double> silenceOnsets;

    for (const auto& frame : spec) {
        double sum = 0.0;
        for (double magnitude : frame) {
            sum += magnitude;
        }

        energies.push_back(sum);
    }

    // TODO: only detect silence after a note has already been played
    int silentFrames = 0;
    bool silencePeriod = false;
    for (int i = 0; i < energies.size(); i++) {
        if (energies[i] < silenceThreshold) {
            silentFrames++;
            if (silentFrames >= minSilenceFrames && !silencePeriod) {
                silencePeriod = true;
                silenceOnsets.push_back(i);
            } 
        } else {
            silentFrames = 0;
            silencePeriod = false;
        }
    }

    return silenceOnsets;
}
