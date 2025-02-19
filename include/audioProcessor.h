#include <string>
#include <vector>

#ifndef AUDIOPROCESSOR_H
#define AUDIOPROCESSOR_H

class AudioProcessor{
public:
    virtual ~AudioProcessor() = default;

    virtual std::vector<double> process() = 0;
};

#endif // AUDIOPROCESSOR_H