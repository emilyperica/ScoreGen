#include <fftw3.h>
#include <math.h>
#include <string>
#include <iostream>

#define MAG_THRESHOLD 50

std::string getNote(double freq);

void detectPitch(float *buf, int frames, int sample_rate) 
{
    static std::string prev_note = "";

    std::string curr_note = "";
    double max_mag = 0.0;
    double dominant_freq = 0.0;

    /* Perform FFT on time-domain data. */
    fftw_complex *in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * frames);
    fftw_complex *out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * frames);
    fftw_plan plan = fftw_plan_dft_1d(frames, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    /* Initialize FFT input array complex numbers, execute the transform. */
    for (int i = 0; i < frames; i++) 
    {
        in[i][0] = buf[i];
        in[i][1] = 0.0;
    }
    fftw_execute(plan);

    /* Iterate over frequency bins, identify the dominant frequency present in the samples. */
    for (int i = 0; i < frames / 2; i++) 
    {
        double magnitude = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        double frequency = i * sample_rate / (double)frames;
        if (magnitude > MAG_THRESHOLD && magnitude > max_mag)
        {
            max_mag = magnitude;
            dominant_freq = frequency;
        }
    }
    if (dominant_freq > 0.0) 
    {
        curr_note = getNote(dominant_freq);
    }
    
    /* Only display detected notes that are new. */
    if (!curr_note.empty() && curr_note != prev_note) 
    {
        printf("Dominant Frequency: %f Hz, Magnitude: %f\n", dominant_freq, max_mag);
        std::cout << "Note: " << curr_note << std::endl;
        prev_note = curr_note;
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
}
/* Function to map frequency in Hz to corresponding note and octave. */
std::string getNote(double freq)
{
    int A4 = 440;
    float C0 = A4 * pow(2, -4.75);
    std::string name[12] = {"C", "C#", "D", "D#","E", "F", "F#", "G", "G#", "A", "A#", "B"};

    int h = round(12 * log2(freq / C0));
    int octave = h / 12;
    int n = h % 12;

    return name[n] + std::to_string(octave);
}