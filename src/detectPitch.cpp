#include <fftw3.h>
#include <math.h>
#include <string.h>
#include <iostream>

using namespace std;

#define MAG_THRESHOLD 100

string getNote(double freq);

void detectPitch(float *buf, int frames, int sample_rate) 
{
    // prepare FFT arrays that will store the resulting complex numbers
    fftw_complex *in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * frames);
    fftw_complex *out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * frames);
    // specify FFT to be performed on the amount of samples (i.e. frames)
    fftw_plan plan = fftw_plan_dft_1d(frames, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    // transfer time-domain data from buffer to FFT input array, initialize as complex nums
    for (int i = 0; i < frames; i++) {
        in[i][0] = buf[i];
        in[i][1] = 0.0;
    }
    fftw_execute(plan);

    // iterate over frequency bins, identify bins with large magnitudes
    for (int i = 0; i < frames / 2; i++) 
    {
        double magnitude = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        double frequency = i * sample_rate / (double)frames;
        if (magnitude > MAG_THRESHOLD)
        {
            //printf("Frequency: %f Hz, Magnitude: %f\n", frequency, magnitude);
            cout << getNote(frequency) << endl;
        }
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
}

string getNote(double freq)
{
    int A4 = 440;
    float C0 = A4*pow(2,-4.75);
    string name[12] = {"C", "C#", "D", "D#","E", "F", "F#", "G", "G#", "A", "A#", "B"};

    int h = round(12*log2(freq/C0));
    int octave = h/12;
    int n = h%12;

    return name[n] + to_string(octave);
}