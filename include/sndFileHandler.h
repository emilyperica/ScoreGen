#include <string>
#include <vector>
#include <sndfile.h>

#ifndef SNDFILEHANDLER_H
#define SNDFILEHANDLER_H

#define BUFFER_LEN 1024

class SndFileHandler{
public: 
    SndFileHandler(const std::string& sndInputPath, const std::string& sndOutputPath);
    ~SndFileHandler();
private: 
    const std::string sndInputPath_;
    const std::string sndOutputPath_;
    
    std::tuple<SF_INFO, std::vector<double>> readSndFile();
    void writeSndFile(SF_INFO info, std::vector<double> processedData);
    std::vector<std::vector<double>> frameData(const std::vector<double>& data);
};

#endif // SNDFILEHANDLER_H