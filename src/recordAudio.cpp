/* This file was adapted from the PortAudio example file paex_record.c by Phil Burk */
#include "recordAudio.h"

#define FRAMES_PER_BUFFER (64)
#define NUM_CHANNELS      (2)
#define PA_SAMPLE_TYPE    paFloat32
#define SAMPLE_SILENCE    (0.0f)
typedef float SAMPLE;

std::atomic<bool> isRecording(false);
std::atomic<bool> exitRequested(false); // eventually will be moved to application loop in main


static void getUserInput() {
    while (!exitRequested) {
#ifdef _WIN32
        if (_kbhit()) {
            char ch = _getch(); // Get the character
            if (ch == 'r') { // Start/stop recording on 'r'
                // TODO: block this if the callback function is in use
                isRecording = !isRecording;
            }
        }
#else
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt); // Get terminal attributes
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply changes

        fd_set set;
        struct timeval timeout;

        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);

        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 0.1 seconds

        if (select(STDIN_FILENO + 1, &set, nullptr, nullptr, &timeout) > 0) {
            char ch;
            read(STDIN_FILENO, &ch, 1);
            if (ch == 'r') { // Start/stop recording on 'r'
                keepRecording = false;
            }
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore original settings
#endif
    }
}

static void saveAsWav(std::vector<SAMPLE> &recordedSamples, int sampleRate, int numChannels, const char *filename) {
    SF_INFO sfinfo;
    sfinfo.samplerate = sampleRate;
    sfinfo.channels = numChannels;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16; // WAV format, 16-bit PCM

    // Open the WAV file for writing
    SNDFILE *outfile = sf_open(filename, SFM_WRITE, &sfinfo);
    if (!outfile) {
        printf("Error: Could not open file for writing: %s", sf_strerror(nullptr)); fflush(stdout);
        return;
    }

    // Normalize samples to ensure consistent volume
    float maxAmplitude = 0.0f;
    for (SAMPLE sample : recordedSamples) {
        maxAmplitude = std::max(maxAmplitude, std::abs(sample));
    }

    float normalizationFactor = maxAmplitude > 0 ? 1.0f / maxAmplitude : 1.0f;
    for (SAMPLE &sample : recordedSamples) {
        sample *= normalizationFactor;
    }

    // Convert float samples to int16_t for writing to WAV
    std::vector<int16_t> intSamples(recordedSamples.size());
    for (size_t i = 0; i < recordedSamples.size(); ++i) {
        intSamples[i] = static_cast<int16_t>(recordedSamples[i] * 32767.0f);
    }

    // Write the audio data to the file
    int frames = intSamples.size() / numChannels;
    sf_count_t writtenFrames = sf_writef_short(outfile, intSamples.data(), frames);
    if (writtenFrames != frames) {
        printf("Error: Could not write all frames to file."); fflush(stdout);
    }

    // Close the file
    sf_close(outfile);
    printf("\nAudio saved to %s in the working directory.\n", filename); fflush(stdout);
}

static int recordCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData) {
    audioData *data = (audioData *)userData;
    const SAMPLE *rptr = (const SAMPLE *)inputBuffer;
    
    size_t requiredSize = (data->frameIndex + framesPerBuffer) * NUM_CHANNELS;
    data->recordedSamples.resize(requiredSize);
    
    SAMPLE *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    if (inputBuffer == NULL) {
        for (unsigned long i = 0; i<framesPerBuffer; i++) {
            *wptr++ = SAMPLE_SILENCE;
            if (NUM_CHANNELS == 2) *wptr++ = SAMPLE_SILENCE;
        }
    } else {
        for (unsigned long i = 0; i<framesPerBuffer; i++) {
            *wptr++ = *rptr++;
            if (NUM_CHANNELS == 2) *wptr++ = *rptr++;
        }
    }

    data->frameIndex += framesPerBuffer;
    if (!isRecording) return paComplete;

    return paContinue;
}

static int playCallback(const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *userData) {
    audioData *data = (audioData *) userData;
    SAMPLE *wptr = (SAMPLE *) outputBuffer;
    const SAMPLE *rptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    unsigned long framesLeft = data->recordedSamples.size() / NUM_CHANNELS - data->frameIndex;
    unsigned long framesToCopy = (framesLeft < framesPerBuffer) ? framesLeft : framesPerBuffer;
    if (framesLeft <= framesPerBuffer) {
        return paComplete; // Stop the stream
    }

    for (unsigned long i = 0; i < framesToCopy; i++) {
        *wptr++ = *rptr++;
        if (NUM_CHANNELS == 2) *wptr++ = *rptr++;
    }

    data->frameIndex += framesToCopy;
    return paContinue;
}

int recordAudio(void) {
    PaStreamParameters inputParameters, outputParameters;
    const PaDeviceInfo *inputDeviceInfo, *outputDeviceInfo;
    PaStream*          stream;
    PaError            err = paNoError;
    audioData          data;
    std::thread        inputThread;

    data.frameIndex = 0;

    // Suppress debug output from calling Pa_Initialize()
#ifdef _WIN32
    freopen("NUL", "w", stderr);
#else
    freopen("/dev/null", "w", stderr);
#endif

    err = Pa_Initialize();

    // Restore original output
#ifdef _WIN32
    freopen("CON", "w", stderr);
#else
    freopen("/dev/tty", "w", stderr);
#endif

    if (err != paNoError) goto done;

    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        goto done;
    }

    inputDeviceInfo = Pa_GetDeviceInfo(inputParameters.device);
    inputParameters.channelCount = inputDeviceInfo->maxInputChannels >= 2 ? 2 : 1;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = inputDeviceInfo->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    printf("Input device: %s\nMax input channels: %i\nDefault sample rate: %.2f\n", 
            inputDeviceInfo->name, inputDeviceInfo->maxInputChannels, inputDeviceInfo->defaultSampleRate);
    fflush(stdout);

    // Record some audio
    err = Pa_OpenStream(
              &stream,
              &inputParameters,
              NULL,
              inputDeviceInfo->defaultSampleRate,
              FRAMES_PER_BUFFER,
              paClipOff,
              recordCallback,
              &data);
    if (err != paNoError) goto done;

    printf("\n=== Ready to record!! Press 'R' to start. ===\n"); fflush(stdout);
    // Start the keyboard monitoring thread
    inputThread = std::thread(getUserInput);
    while (!isRecording) {
        if (exitRequested) break;
        Pa_Sleep(100);
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) goto done;
    printf("\n=== Now recording!! Press 'R' to stop. ===\n"); fflush(stdout);

    while (isRecording) {
        if (exitRequested) break;
        Pa_Sleep(100);
    }

    err = Pa_StopStream(stream);
    if (err != paNoError) goto done;

    err = Pa_CloseStream(stream);
    if (err != paNoError) goto done;

    exitRequested = true;
    inputThread.join();

    // Playback recorded data
    data.frameIndex = 0;

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default output device.\n");
        goto done;
    }

    outputDeviceInfo = Pa_GetDeviceInfo(outputParameters.device);
    outputParameters.channelCount = outputDeviceInfo->maxOutputChannels >= 2 ? 2 : 1;
    outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = outputDeviceInfo->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    printf("Output device: %s\nMax output channels: %i\nDefault sample rate: %.2f\n", 
            outputDeviceInfo->name, outputDeviceInfo->maxOutputChannels, outputDeviceInfo->defaultSampleRate);
    fflush(stdout);
    saveAsWav(data.recordedSamples, outputDeviceInfo->defaultSampleRate, outputParameters.channelCount);
    printf("\n=== Now playing back. ===\n"); fflush(stdout);
    err = Pa_OpenStream(
              &stream,
              NULL,
              &outputParameters,
              outputDeviceInfo->defaultSampleRate,
              FRAMES_PER_BUFFER,
              paClipOff,
              playCallback,
              &data);
    if (err != paNoError) goto done;

    if (stream) {
        err = Pa_StartStream(stream);
        if (err != paNoError) goto done;

        printf("Waiting for playback to finish.\n"); fflush(stdout);

        while ((err = Pa_IsStreamActive(stream)) == 1) Pa_Sleep(100);
        if (err < 0) goto done;

        err = Pa_CloseStream(stream);
        if (err != paNoError) goto done;

        printf("Done.\n"); fflush(stdout);
    }

done:
    Pa_Terminate(); // TODO: if error ocurred during Pa_Initialize, this should not be called
    if (err != paNoError) {
        fprintf(stderr, "An error occurred while using the portaudio stream\n");
        fprintf(stderr, "Error number: %d\n", err);
        fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
        err = 1; // Always return 0 or 1
    }

    return err;
}