#include "dsp.h"
#include "processJson.h"

#define SILENCE_LENGTH 512
#define defaultBPM 60
#define PPQ 480 // Pulses per quarter note, default for MusicXML
const float TIME_THRESHOLD = 0.1f; // 0.05 seconds threshold for intersection


std::vector<float> prependSilence(const std::vector<float>& buf, size_t silenceLength) {
    std::vector<float> paddedBuffer(silenceLength, 0.0f); // Add silence
    paddedBuffer.insert(paddedBuffer.end(), buf.begin(), buf.end());
    return paddedBuffer;
}

XMLNote convertSingleNote(const Note& note, double startTime, double endTime) {
    XMLNote xmlNote;
    int octave = 0;
    int alter = 0;
    std::string noteName;

    // Convert note duration in seconds to divisions
    double noteDurationInSeconds = endTime - startTime;
    xmlNote.duration = std::round(noteDurationInSeconds * (defaultBPM / 60.0) * PPQ);

    xmlNote.type = note.type;

    // If it's a rest, mark it and return
    if (note.pitch == "Rest") {
        xmlNote.isRest = true;
        return xmlNote;
    }

    // Extract note name, octave, and alteration
    for (char c : note.pitch) {
        if (std::isalpha(c) && c != 'b') {
            noteName += c;
        }
        else if (c == '#' || c == 'b') {
            alter = (c == '#') ? 1 : -1;
        }
        else if (std::isdigit(c)) {
            octave = octave * 10 + (c - '0');
        }
    }

    xmlNote.pitch = noteName;
    xmlNote.octave = octave;
    xmlNote.alter = alter;
    xmlNote.staff = note.staff;

    return xmlNote;
}

// Convert Note to XMLNote (monophonic conversion)
XMLNote convertToXMLNote(const Note& note) {
    XMLNote xmlNote;
    int octave = 0;
    int alter = 0;
    std::string noteName;

    // Convert note duration in seconds to duration in divisions
    float noteDurationInSeconds = note.endTime - note.startTime;
    xmlNote.duration = static_cast<int>(std::round((noteDurationInSeconds * (defaultBPM / 60.0f) * PPQ)));
    xmlNote.type = note.type;
    xmlNote.startTime = static_cast<int>(std::round(note.startTime * (defaultBPM / 60.0f) * PPQ)); // Start time in divisions
    xmlNote.staff = note.staff;

    // Extract note name and octave
    if (note.pitch == "Rest") {
        xmlNote.isRest = true;
        xmlNote.pitch = "";
        xmlNote.alter = 0;
        xmlNote.octave = 0;
        return xmlNote;
    }


    for (char c : note.pitch) {
        if (std::isalpha(c) && c != 'b') {
            noteName += c;
        }
        else if (c == '#' || c == 'b') {
            alter = (c == '#') ? 1 : -1;
        }
        else if (std::isdigit(c)) {
            octave = octave * 10 + (c - '0');
        }
    }

    xmlNote.pitch = noteName;
    xmlNote.octave = octave;
    xmlNote.alter = alter;
    xmlNote.isRest = false;

    return xmlNote;
}

void findIntersectingNotes(std::vector<XMLNote>& xmlNotes) {
    size_t i = 0;
    while (i < xmlNotes.size()) {
        std::vector<XMLNote*> trebleNotes;
        std::vector<XMLNote*> bassNotes;

        // Separate notes into treble and bass
        for (; i < xmlNotes.size(); ++i) {
            if (xmlNotes[i].staff == Staff::Treble)
                trebleNotes.push_back(&xmlNotes[i]);
            else
                bassNotes.push_back(&xmlNotes[i]);
        }

        // Sort and process treble clef
        std::sort(trebleNotes.begin(), trebleNotes.end(), [](const XMLNote* a, const XMLNote* b) {
            return a->startTime < b->startTime;
            });

		cout << TIME_THRESHOLD * PPQ << endl;

        for (size_t k = 0; k < trebleNotes.size(); ++k) {
            trebleNotes[k]->voice = 1;
            trebleNotes[k]->isChord = (k > 0) &&
                (trebleNotes[k - 1]->startTime <= trebleNotes[k]->startTime &&
                    trebleNotes[k - 1]->startTime + trebleNotes[k - 1]->duration > trebleNotes[k]->startTime + TIME_THRESHOLD*PPQ) &&
                    trebleNotes[k]->startTime - trebleNotes[k-1]->startTime <= TIME_THRESHOLD*PPQ;
        }

        // Sort and process bass clef
        std::sort(bassNotes.begin(), bassNotes.end(), [](const XMLNote* a, const XMLNote* b) {
            return a->startTime < b->startTime;
            });

        for (size_t k = 0; k < bassNotes.size(); ++k) {
            bassNotes[k]->voice = 2;
            bassNotes[k]->isChord = (k > 0) &&
                (bassNotes[k - 1]->startTime <= bassNotes[k]->startTime + TIME_THRESHOLD * PPQ &&
                    bassNotes[k - 1]->startTime + bassNotes[k - 1]->duration > bassNotes[k]->startTime + TIME_THRESHOLD*PPQ) &&
                    bassNotes[k]->startTime - bassNotes[k-1]->startTime <= TIME_THRESHOLD*PPQ;
        }
    }
}

// Convert a list of Notes to XMLNotes with polyphonic handling
std::vector<XMLNote> convertToPolyphonicXMLNotes(const std::vector<Note>& notes) {
    std::vector<XMLNote> xmlNotes;

    // Convert all notes to XMLNotes
    for (const auto& note : notes) {
        xmlNotes.push_back(convertToXMLNote(note));
    }


    // Find intersecting notes and mark chords
    findIntersectingNotes(xmlNotes);

    return xmlNotes;
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
    std::vector<Note> notes = processNotes("note_events.json", defaultBPM);
	std::vector<XMLNote> xmlNotes = convertToPolyphonicXMLNotes(notes);
    for (const Note& note : notes) {
        cout << "Note: " << note.pitch << "\tDuration: " << note.endTime - note.startTime << "\tNote Type: " << note.type << endl;
    }
    result.XMLNotes = xmlNotes;

    // Write the first 10 notes to a text file
    std::ofstream outFile("XmlNotes.txt");
    if (outFile.is_open()) {
        for (size_t i = 0; i < result.XMLNotes.size(); ++i) {
            const XMLNote& xmlNote = result.XMLNotes[i];
            outFile << "Pitch: " << xmlNote.pitch << ", "
                << "Alter: " << xmlNote.alter << ", "
                << "Octave: " << xmlNote.octave << ", "
                << "Start Time: " << xmlNote.startTime << ", "
                << "Duration: " << xmlNote.duration << ", "
                << "Type: " << xmlNote.type << ", "
                << "Is Rest: " << (xmlNote.isRest ? "true" : "false") << ", "
                << "Is Chord: " << (xmlNote.isChord ? "true" : "false") << ", "
                << "Voice: " << xmlNote.voice << ", " << std::endl;
        }
        outFile.close();
    }
    else {
        std::cerr << "Unable to open file for writing." << std::endl;
    }

    // Extract key signature
    std::vector<int> durations = calculatePitchDurations(result.XMLNotes);
    std::string detectedKey = findKey(durations);
    result.keySignature = convertToKeySignature(detectedKey);

    return result;
}