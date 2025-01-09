#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <atomic>
#include <vector>

#include "portaudio.h"
#include "sndfile.h"

#ifndef RECORDAUDIO_H
#define RECORDAUDIO_H

#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"

typedef struct
{
    int frameIndex;
    std::vector<SAMPLE> recordedSamples;
}
audioData;

static void getUserInput();
static void saveAsWav(std::vector<SAMPLE> &recordedSamples, int sampleRate, int numChannels, const char *filename = "recorded.wav");
static int recordCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData);
static int playCallback(const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *userData );
int recordAudio(void);

#endif