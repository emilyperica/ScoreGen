#include "audioProcessor.h"

#ifndef CHANNELCONVERTER_H
#define CHANNELCONVERTER_H

class ChannelConverter : public AudioProcessor{
    public:
        ChannelConverter(const int& channels, const std::vector<double>& data);
        ~ChannelConverter() override;

        std::vector<double> process() override;

    private:
        const int channels_;
        const std::vector<double> data_;
};

#endif // CHANNELCONVERTER_H