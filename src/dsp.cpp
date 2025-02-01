#include "dsp.h"

#define SILENCE_LENGTH 512
#define defaultBPM 60

std::vector<float> prependSilence(const std::vector<float>& buf, size_t silenceLength) {
    std::vector<float> paddedBuffer(silenceLength, 0.0f); // Add silence
    paddedBuffer.insert(paddedBuffer.end(), buf.begin(), buf.end());
    return paddedBuffer;
}

void dsp(const char* infilename) {
    SNDFILE* infile;
    SF_INFO sfinfo;
    vector<float> buf;
    
	memset(&sfinfo, 0, sizeof(sfinfo));
    if (!(infile = sf_open(infilename, SFM_READ, &sfinfo))) {
		printf("Not able to open requested file %s.\n", infilename) ;
		puts(sf_strerror(NULL));
		return;
	}

    size_t numFrames = sfinfo.frames * sfinfo.channels;
    vector<float> tempBuffer(numFrames);
    sf_count_t numRead = sf_read_float(infile, tempBuffer.data(), numFrames);
    sf_close(infile);

    // Convert interleaved audio to mono if necessary
    if (sfinfo.channels > 1) {
        buf.resize(sfinfo.frames);
        for (size_t i = 0; i < sfinfo.frames; ++i) {
            float sum = 0.0f;
            for (int ch = 0; ch < sfinfo.channels; ++ch) {
                sum += tempBuffer[i * sfinfo.channels + ch];
            }

            buf[i] = sum / sfinfo.channels;
        }
    } else {
        buf = move(tempBuffer);
    }

    std::vector<float> paddedBuf = prependSilence(buf, SILENCE_LENGTH);

    vector<Note> notes = detectNotes(paddedBuf, sfinfo.samplerate, sfinfo.channels, defaultBPM);
    for (Note note : notes) {
        cout << "Note: " << note.pitch << "\tDuration: " << note.endTime-note.startTime << "\tNote Type: " << note.type << endl;
    }
}