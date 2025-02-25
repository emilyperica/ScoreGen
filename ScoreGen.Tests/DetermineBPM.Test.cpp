#include <gtest/gtest.h>
#include <vector>
#include <sndfile.h>
#include <iostream>
#include <filesystem>
#include "determineBPM.h"

TEST(CalculateMedianTest, HandlesEmptyVector) {
    std::vector<float> values;
    EXPECT_EQ(calculateMedian(values), 0.0f);
}

TEST(CalculateMedianTest, HandlesSingleElement) {
    std::vector<float> values = { 42.0f };
    EXPECT_EQ(calculateMedian(values), 42.0f);
}

TEST(CalculateMedianTest, HandlesOddNumberOfElements) {
    std::vector<float> values = { 1.0f, 3.0f, 2.0f };
    EXPECT_EQ(calculateMedian(values), 2.0f);
}

TEST(CalculateMedianTest, HandlesEvenNumberOfElements) {
    std::vector<float> values = { 1.0f, 3.0f, 2.0f, 4.0f };
    EXPECT_EQ(calculateMedian(values), 2.5f);
}

class BPMDetectionTest : public ::testing::TestWithParam<std::tuple<const char*, float>> {
protected:
    std::vector<float> loadAudioFile(const char* filename) {
        SF_INFO sfinfo;
        memset(&sfinfo, 0, sizeof(sfinfo));

        SNDFILE* file = sf_open(filename, SFM_READ, &sfinfo);
        if (!file) {
            throw std::runtime_error("Could not open audio file");
        }

        std::vector<float> buffer(sfinfo.frames * sfinfo.channels);
        sf_readf_float(file, buffer.data(), sfinfo.frames);
        sf_close(file);

        return buffer;
    }
};

struct TestCase {
    const char* filepath;
    float expectedBPM;

    TestCase(const char* path, float bpm) : filepath(path), expectedBPM(bpm) {}
};

std::vector<TestCase> testCases = {
    {"piano-samples/sample-scales/c-major-scale-descending-on-bass-clef.wav", 120.0f},
    {"piano-samples/sample-scales/c-major-scale-on-bass-clef.wav", 120.0f},
    {"piano-samples/sample-scales/c-major-scale-descending-on-treble-clef.wav", 120.0f},
    {"piano-samples/sample-scales/c-major-scale-on-treble-clef.wav", 120.0f}
};

std::vector<std::tuple<const char*, float>> convertTestCases(const std::vector<TestCase>& testCases) {
    std::vector<std::tuple<const char*, float>> result;
    for (const auto& testCase : testCases) {
        result.emplace_back(testCase.filepath, testCase.expectedBPM);
    }
    return result;
}

TEST_P(BPMDetectionTest, DetectsBPMForDifferentFiles) {
    auto [filepath, expectedBPM] = GetParam();
    std::vector<float> buffer = loadAudioFile(filepath);

    // Test default mode
    std::map<std::string, std::string> params{ {"mode", "default"} };
    float bpm = getBufferBPM(buffer, 44100, params);
    EXPECT_NEAR(bpm, expectedBPM, 5.0f);
}

INSTANTIATE_TEST_SUITE_P(
    BPMTests,
    BPMDetectionTest,
    testing::ValuesIn(convertTestCases(testCases))
);

// Additional specific mode tests
TEST_F(BPMDetectionTest, HandlesEmptyBuffer) {
    std::vector<float> buffer;
    std::map<std::string, std::string> params{ {"mode", "default"} };
    float bpm = getBufferBPM(buffer, 44100, params);
    EXPECT_EQ(bpm, 0.0f);
}

TEST_F(BPMDetectionTest, InvalidMode) {
    std::vector<float> buffer = loadAudioFile(testCases[0].filepath);
    std::map<std::string, std::string> params{ {"mode", "invalid"} };
    EXPECT_THROW(getBufferBPM(buffer, 44100, params), std::invalid_argument);
}