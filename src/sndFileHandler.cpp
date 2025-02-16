#include "sndFileHandler.h"

#define FRAME_SIZE 2048

SndFileHandler::SndFileHandler(const std::string& sndInputPath, const std::string& sndOutputPath)
    : sndInputPath_(sndInputPath), sndOutputPath_(sndOutputPath){}

SndFileHandler::~SndFileHandler(){}

std::tuple<SF_INFO, std::vector<double>> SndFileHandler::readSndFile(){
    SF_INFO sfInfo;
    int readCount;
    double buffer[FRAME_SIZE];

    SNDFILE* inputFile = sf_open(sndInputPath_.c_str(), SFM_READ, &sfInfo);
    if (!inputFile){
        printf("Error: Cannot open input file -- %s\n", sf_strerror(inputFile));
        return std::make_tuple(sfInfo, std::vector<double>());
    }

    printf("Opened file: %s, now reading...\n", sndInputPath_.c_str());
    std::vector<double> data;
    while ((readCount = sf_readf_double(inputFile, buffer, FRAME_SIZE))){
        for (int i = 0; i < readCount; i++){
            data.push_back(buffer[i]);
        }
        }
        printf("Finished reading file. Total frames read: %zu\n", data.size());

    sf_close(inputFile);
    
    return std::make_tuple(sfInfo, data);
}

void SndFileHandler::writeSndFile(SF_INFO info, std::vector<double> data){

    SNDFILE* outputFile = sf_open(sndOutputPath_.c_str(), SFM_WRITE, &info);
    if (!outputFile){
        printf("Error: Cannot open output file -- %s\n", sf_strerror(outputFile));
        return;
    }
    sf_write_double(outputFile, data.data(), data.size());
    sf_close(outputFile);
}