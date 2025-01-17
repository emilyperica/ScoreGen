#include "dsp.h"

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

    vector<Note> notes = detectNotes(buf, sfinfo.samplerate, sfinfo.channels);
    for (Note note : notes) {
        cout << "Note: " << note.pitch << "\tDuration: " << note.endTime-note.startTime << endl;
    }
}