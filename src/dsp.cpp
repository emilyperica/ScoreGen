#include "dsp.h"
#include "generateMusicXML.h"

#define SILENCE_LENGTH 512
#define DIVISIONS 480

using namespace std;


std::vector<float> prependSilence(const std::vector<float>& buf, size_t silenceLength) {
    std::vector<float> paddedBuffer(silenceLength, 0.0f); // Add silence
    paddedBuffer.insert(paddedBuffer.end(), buf.begin(), buf.end());
    return paddedBuffer;
}

// Function to convert vector of Note to vector of xmlNote
std::vector<xmlNote> convertToXmlNotes(const std::vector<Note>& notes) {
    std::vector<xmlNote> xmlNotes;
    xmlNotes.reserve(notes.size());

    for (const auto& note : notes) {
        xmlNote xNote;
		if (note.pitch == "rest") {
			xNote.isRest = true;
		}
		else if (note.pitch.length() == 3) {
			xNote.pitch = note.pitch.substr(0, 1);
			xNote.alter = note.pitch[1] == '#' ? 1 : -1;
			xNote.octave = std::stoi(note.pitch.substr(2, 1));
		}
		else {
			xNote.pitch = note.pitch.substr(0,1);
			xNote.octave = std::stoi(note.pitch.substr(1, 1));
		}
        xNote.type = note.type;
        xNote.duration = static_cast<int>(note.endTime - note.startTime);
        xmlNotes.push_back(xNote);
    }

    return xmlNotes;
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

    vector<Note> notes = detectNotes(paddedBuf, sfinfo.samplerate, sfinfo.channels);
	vector<xmlNote> xmlNotes = convertToXmlNotes(notes);
	MusicXMLGenerator generator;
	bool generated = generator.generate(xmlNotes, "output.xml", DIVISIONS);

    for (Note note : notes) {
        cout << "Note: " << note.pitch << "\tDuration: " << note.endTime-note.startTime << "\tNote Type: " << note.type << endl;
    }
}