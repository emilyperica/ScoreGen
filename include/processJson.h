#ifndef PROCESS_NOTES_H
#define PROCESS_NOTES_H

#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <string>

std::vector<Note> processNotes(const std::string& jsonPath, int bpm);

#endif // PROCESS_NOTES_H
