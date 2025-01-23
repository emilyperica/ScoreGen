#ifndef NOTE_DETECTION_H
#define NOTE_DETECTION_H

#include <string>
#include <vector>
#include <map>

// Constants for pitch and frequency
#define A4 440
#define C0 (A4 * pow(2, -4.75))

// Structure representing a musical note
struct Note {
    float startTime;
    float endTime;
    std::string pitch;
    std::string type;
};

// Function declarations
std::string getNoteName(double freq);
std::string determineNoteType(float noteDuration, int bpm);
std::vector<Note> detectNotes(const std::vector<float>& buf, int sample_rate, int channels);
std::vector<float> prependSilence(const std::vector<float>& buf, size_t silenceLength);
float calculateMedian(const std::vector<float>& values);
float getBufferBPM(const std::vector<float>& buf, int sample_rate, const std::map<std::string, std::string>& params = {});

// External dependencies
extern "C" int __cdecl __ms_vsnprintf(char* buffer, size_t size, const char* format, va_list args);
extern "C" FILE* __cdecl __iob_func(void);

#endif // NOTE_DETECTION_H
