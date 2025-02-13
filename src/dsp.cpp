#include "dsp.h"
#include "determineBPM.h"

#define SILENCE_LENGTH 512
#define PPQ 480 // Pulses per quarter note, default for MusicXML

std::vector<float> prependSilence(const std::vector<float>& buf, size_t silenceLength) {
    std::vector<float> paddedBuffer(silenceLength, 0.0f); // Add silence
    paddedBuffer.insert(paddedBuffer.end(), buf.begin(), buf.end());
    return paddedBuffer;
}

XMLNote convertToXMLNote(const Note& note, int bpm) {
    XMLNote xmlNote;
    int octave = 0;
    int alter = 0;
    std::string noteName;

    // Extract note name and octave
    if (note.pitch == "Rest") {
        xmlNote.isRest = true;
        return xmlNote;
    }

    for (char c : note.pitch) {
        if ((std::isalpha(c) == 1) && (c != 'b')) {
            noteName += c;
        } 
        else if (c == '#' || c == 'b') {
            alter = c == '#' ? 1 : -1;
        }
        else if (std::isdigit(c)) {
            octave = octave * 10 + (c - '0');
        }
    }

    // Convert note duration in s to duration in divisions
    float noteDurationInSeconds = note.endTime - note.startTime;
    xmlNote.duration = static_cast<int>(std::round((noteDurationInSeconds * (bpm / 60.0) * PPQ) / PPQ) * PPQ);

    xmlNote.pitch = noteName;
    xmlNote.octave = octave;
    xmlNote.alter = alter;
    xmlNote.type = note.type;
    xmlNote.isRest = false;

    return xmlNote;
}

std::vector<int> calculatePitchDurations(const std::vector<XMLNote>& xmlNotes) {
    std::vector<int> durations(12, 0);
    std::map<std::string, int> pitchClasses = 
    {
        {"C", 0}, {"C#", 1}, {"D", 2}, {"D#", 3}, 
        {"E", 4},{"F", 5}, {"F#", 6}, {"G", 7}, 
        {"G#", 8},{"A", 9}, {"A#", 10}, {"B", 11}
    };
    
    for (const auto& xmlNote : xmlNotes) 
    {
        int pitchClass = pitchClasses[xmlNote.pitch];
        durations[pitchClass] += xmlNote.duration;
    }
    
    return durations;
}

int convertToKeySignature(const string& key) {
    static const map<string, int> keyToSignature = 
    {
        // Major keys
        {"C", 0},  {"G", 1},  {"D", 2},   {"A", 3}, 
        {"E", 4},  {"B", 5},  {"F#", 6},  {"C#", 7},
        {"F", -1}, {"Bb", -2}, {"Eb", -3}, {"Ab", -4},
        {"Db", -5}, {"Gb", -6}, {"Cb", -7},
        
        // Minor keys
        {"a", 0},  {"e", 1},  {"b", 2},   {"f#", 3},
        {"c#", 4}, {"g#", 5}, {"d#", 6},  {"a#", 7},
        {"d", -1}, {"g", -2}, {"c", -3},  {"f", -4},
        {"bb", -5}, {"eb", -6}, {"ab", -7}
    };
    auto it = keyToSignature.find(key);
    return (it != keyToSignature.end()) ? it->second : 0;  // Default to C major
}

DSPResult dsp(const char* infilename) {
    DSPResult result;

    SNDFILE* infile;
    SF_INFO sfinfo;
    vector<float> buf;
    
	memset(&sfinfo, 0, sizeof(sfinfo));
    if (!(infile = sf_open(infilename, SFM_READ, &sfinfo))) {
		printf("Not able to open requested file %s.\n", infilename) ;
		puts(sf_strerror(NULL));
        exit(EXIT_FAILURE);
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

    // Extract notes
    std::vector<Note> notes = detectNotes(paddedBuf, sfinfo.samplerate, sfinfo.channels);
    int bpm = getBufferBPM(paddedBuf, sfinfo.samplerate);
    for (const Note& note : notes) {
        result.XMLNotes.push_back(convertToXMLNote(note, bpm));
    }

    // Extract key signature
    std::vector<int> durations = calculatePitchDurations(result.XMLNotes);
    std::string detectedKey = findKey(durations);
    result.keySignature = convertToKeySignature(detectedKey);

    return result;
}