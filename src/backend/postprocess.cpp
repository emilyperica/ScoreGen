#include "postprocess.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <stdexcept>
#include <string>

using namespace std;

string readFile(const string& filePath)
{
    ifstream file(filePath);
    if (!file.is_open()) {
        throw runtime_error("Cannot open input file: " + filePath);
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void writeFile(const string& filePath, const string& content)
{
    ofstream file(filePath);
    if (!file.is_open()) {
        throw runtime_error("Cannot open output file: " + filePath);
    }
    file << content;
    file.close();
}

void postProcessMusicXML(const string& inputPath, const string& outputPath)
{
    // Read the entire input MusicXML file.
    string content = readFile(inputPath);
    
    // Regex to match <note>...</note> blocks (using [\s\S] to match all characters).
    regex noteRegex("<note>([\\s\\S]*?)</note>");
    
    // Lambda to process each <note> block.
    auto processNote = [](const smatch& m) -> string {
        string noteBlock = m.str(); // Entire <note>...</note> block.
        
        // If there's no <pitch> tag, assume it's a rest and insert <rest/>.
        if (noteBlock.find("<pitch>") == string::npos) {
            size_t pos = noteBlock.find("<note>");
            if (pos != string::npos) {
                pos += 6; // position right after "<note>"
                noteBlock.insert(pos, "\n  <rest/>\n");
            }
        }
        else {
            // If a <pitch> tag exists and there's an <alter> but no <accidental>, insert <accidental>.
            if (noteBlock.find("<alter>") != string::npos &&
                noteBlock.find("<accidental>") == string::npos)
            {
                regex alterRegex("<alter>(.*?)</alter>");
                smatch alterMatch;
                if (regex_search(noteBlock, alterMatch, alterRegex)) {
                    string alterVal = alterMatch[1].str();
                    string accidental;
                    if (alterVal == "1")
                        accidental = "sharp";
                    else if (alterVal == "-1")
                        accidental = "flat";
                    else if (alterVal == "2")
                        accidental = "double-sharp";
                    else if (alterVal == "-2")
                        accidental = "flat-flat";
                    // Insert the accidental element immediately after </pitch>
                    size_t pos = noteBlock.find("</pitch>");
                    if (pos != string::npos) {
                        pos += 8; // after "</pitch>"
                        noteBlock.insert(pos, "\n  <accidental>" + accidental + "</accidental>");
                    }
                }
            }
        }
        return noteBlock;
    };

    // Manually iterate over matches and build the new string.
    string processed;
    size_t lastPos = 0;
    for (sregex_iterator iter(content.begin(), content.end(), noteRegex), end; iter != end; ++iter) {
        smatch match = *iter;
        // Append text before this match.
        processed.append(content.substr(lastPos, match.position() - lastPos));
        // Process the matched note block.
        processed.append(processNote(match));
        lastPos = match.position() + match.length();
    }
    // Append any remaining text after the last match.
    processed.append(content.substr(lastPos));

    // Write the processed content to the output file.
    writeFile(outputPath, processed);
    
    // Print a success message.
    cout << "Postprocessing complete. Output written to " << outputPath << endl;
}
