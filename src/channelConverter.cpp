#include <sndfile.h>
#include "channelConverter.h"

ChannelConverter::ChannelConverter(const int& channels, const std::vector<double>& data)
    : channels_(channels), data_(data){}

ChannelConverter::~ChannelConverter(){}

std::vector<double> ChannelConverter::process(){

    std::vector<double> convertedData(data_.size() / channels_);

    // Sum the signal among interleaved samples, average to mono
    for (int i = 0; i < data_.size(); i += channels_){
        double sum = 0;
        for (int j = 0; j < channels_; j++){
            sum += data_[i + j];
        }
        convertedData[i / channels_] = sum / channels_;
    }

    return convertedData;
    // Note: still need to change sfInfo.channels to 1
}