#include <string>
#include <vector>
#include <tuple>
#include <sndfile.h>

#ifndef SNDFILEHANDLER_H
#define SNDFILEHANDLER_H

class SndFileHandler{
public: 
    SndFileHandler(const std::string& sndInputPath, const std::string& sndOutputPath);
    ~SndFileHandler();

    std::tuple<SF_INFO, std::vector<double>> readSndFile();
    void writeSndFile(SF_INFO info, std::vector<double> processedData);

private: 
    const std::string sndInputPath_;
    const std::string sndOutputPath_;
};

#endif // SNDFILEHANDLER_H