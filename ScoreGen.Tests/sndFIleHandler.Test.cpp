#define _USE_MATH_DEFINES
#define FP_TOL 1e-4 // slightly reduced bc of sndfile 16 bit PCM which is used elsewhere


#include <gtest/gtest.h>
#include <filesystem>
#include <cmath>
#include "sndFileHandler.h"
#include "test-helpers/test-helpers.h"


TEST(SndFileHandlerTest, ReadValidFile) {
    std::filesystem::create_directory("test-data");

    SF_INFO sfinfo;
    sfinfo.channels = 1.0;
    sfinfo.samplerate = 44100.0;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    SNDFILE* file = sf_open("test-data/sine.wav", SFM_WRITE, &sfinfo);

    std::vector<double> sineWave = generateSineWave(440.0, sfinfo.samplerate, 1.0);

    sf_write_double(file, sineWave.data(), sineWave.size());
    sf_close(file);
    
    SndFileHandler handler("test-data/sine.wav", "");
    auto [sfInfo, data] = handler.readSndFile();
    
    EXPECT_EQ(sfInfo.channels, 1.0);
    EXPECT_EQ(sfInfo.samplerate, 44100.0);
    EXPECT_EQ(data.size(), 44100.0);

    EXPECT_NEAR(data[0], 0.0, FP_TOL);
    EXPECT_NEAR(data[100], sin(2.0 * M_PI * 440.0 * 100.0 / 44100.0), FP_TOL);
    EXPECT_NEAR(data[1234], sin(2.0 * M_PI * 440.0 * 1234.0 / 44100.0), FP_TOL);

    std::filesystem::remove_all("test-data");
}

TEST(SndFileHandlerTest, ReadNonExistentFile) {
    SndFileHandler handler("fakefile.wav", "");
    auto [sfInfo, data] = handler.readSndFile();
    
    EXPECT_EQ(data.size(), 0);
}

TEST(SndFileHandlerTest, ReadWriteRead) {
    std::filesystem::create_directory("test-data");
    SF_INFO sfinfo;
    sfinfo.channels = 1.0;
    sfinfo.samplerate = 44100.0;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    SNDFILE* file = sf_open("test-data/sine.wav", SFM_WRITE, &sfinfo);

    std::vector<double> sineWave = generateSineWave(440.0, sfinfo.samplerate, 1.0);

    sf_write_double(file, sineWave.data(), sineWave.size());
    sf_close(file);
    
    SndFileHandler readHandler("test-data/sine.wav", "");
    auto [originalInfo, originalData] = readHandler.readSndFile();
    
    SndFileHandler writeHandler("", "test-data/output.wav");
    writeHandler.writeSndFile(originalInfo, originalData);
    
    SndFileHandler readAgainHandler("test-data/output.wav", "");
    auto [newInfo, newData] = readAgainHandler.readSndFile();
    
    EXPECT_EQ(originalInfo.channels, newInfo.channels);
    EXPECT_EQ(originalInfo.samplerate, newInfo.samplerate);
    EXPECT_EQ(originalInfo.format, newInfo.format);
    
    EXPECT_EQ(originalData.size(), newData.size());
    bool sameData = true;

    for (int i = 0; i < originalData.size(); i++) {
        if (std::abs(originalData[i] - newData[i]) > FP_TOL) {
            sameData = false;
        }
    }
    EXPECT_TRUE(sameData);
    
    std::filesystem::remove_all("test-data");
}

TEST(SndFileHandlerTest, WriteInvalidLocation) {
    SndFileHandler handler("", "/DNE/fakefile.wav");
    
    SF_INFO sfInfo;
    sfInfo.channels = 1.0;
    sfInfo.samplerate = 44100.0;
    sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    
    std::vector<double> data(1000.0, 0.5);
    
    EXPECT_NO_FATAL_FAILURE(handler.writeSndFile(sfInfo, data));
}