#include "STFT.h"
#include "hammingFunction.h"
#include <iostream>

std::vector<std::vector<double>> STFT(const std::vector<double>& data, int windowSize, int hopSize){

    fftw_complex* in;
    fftw_complex* out;
    fftw_plan planForward;

    // Prepare spectrogram data structure
    std::vector<std::vector<double>> spectrogram;
    spectrogram.reserve(data.size() / hopSize);

    // Allocate memory for FFT in and out
    in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * windowSize);
    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * windowSize);

    // Create a plan
    planForward = fftw_plan_dft_1d(windowSize, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // Create a hamming window
    std::vector<double> hammingWindow = hammingFunction(windowSize);
    std::cout << "First few values of hammingWindow:\n";
    for (int i = 0; i < std::min(windowSize, 10); i++) {
        std::cout << hammingWindow[i] << " ";
    }
    std::cout << std::endl; 

    // Perform STFT
    int chunkPosition = 0;
    int bStop = 0;
    int readIndex;

    while(chunkPosition < data.size() && !bStop){
        for(int i = 0; i < windowSize; i++){
            readIndex = chunkPosition + i;
            if (readIndex < data.size()){
                in[i][0] = data[readIndex] * hammingWindow[i];
                in[i][1] = 0.0;
            }
            else{
                in[i][0] = 0.0;
                in[i][1] = 0.0;
                bStop = 1;
            }
        }
        // Perform FFT on frame
        fftw_execute(planForward);

        if (chunkPosition == 0) {
            std::cout << "First FFT input frame:\n";
            for (int i = 0; i < std::min(windowSize, 10); i++) {
                std::cout << "(" << in[i][0] << ", " << in[i][1] << ") ";
            }
            std::cout << std::endl;

            std::cout << "First FFT output:\n";
            for (int i = 0; i < std::min(windowSize / 2 + 1, 10); i++) {
                std::cout << "(" << out[i][0] << ", " << out[i][1] << ") ";
            }
            std::cout << std::endl;
        } 
        // Add to spectrogram data structure
        // A 2D vector where each row is a time frame and each column is a frequency bin
        // Frequency information at a specific time frame : std::vector<double> spectrogram[timeIndex]
        // To analyze a single frequency over a time : double magnitude = spectrogram[frameIndex][freqIndex];
        std::vector<double> freqMagnitudes;
        freqMagnitudes.reserve(windowSize / 2 + 1);

        for(int i = 0; i < windowSize / 2 + 1; i++){
            freqMagnitudes.push_back(sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]));
        }

        spectrogram.push_back(freqMagnitudes);
        chunkPosition += hopSize;
    }

    // Clean up
    fftw_destroy_plan(planForward);
    fftw_free(in);
    fftw_free(out);

    return spectrogram;
}