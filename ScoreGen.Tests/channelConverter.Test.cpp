#include <gtest/gtest.h>
#include "channelConverter.h"
#include <vector>

TEST(ChannelConverterTest, SingleChannel) {
    std::vector<double> data = {0.5, 1.5, -2.0, 3.0};
    int channels = 1;
    ChannelConverter converter(channels, data);
    std::vector<double> result = converter.process();

    ASSERT_EQ(result.size(), data.size());
    for (size_t i = 0; i < result.size(); ++i) {
        EXPECT_DOUBLE_EQ(result[i], data[i]);
    }
}

TEST(ChannelConverterTest, TwoChannelConversion) {
    std::vector<double> data = {1.0, 2.0, 1.1, 2.1, 1.2, 2.2};
    int channels = 2;
    ChannelConverter converter(channels, data);
    std::vector<double> result = converter.process();

    ASSERT_EQ(result.size(), data.size() / channels);
    EXPECT_DOUBLE_EQ(result[0], (data[0] + data[1]) / channels);
    EXPECT_DOUBLE_EQ(result[1], (data[2] + data[3]) / channels);
    EXPECT_DOUBLE_EQ(result[2], (data[4] + data[5]) / channels);
}

TEST(ChannelConverterTest, ThreeChannelConversion) {
    std::vector<double> data = {1.0, 2.0, 3.0, 1.1, 2.1, 3.1};
    int channels = 3;
    ChannelConverter converter(channels, data);
    std::vector<double> result = converter.process();

    ASSERT_EQ(result.size(), data.size() / channels);
    EXPECT_DOUBLE_EQ(result[0], (data[0] + data[1] + data[2]) / channels);
    EXPECT_DOUBLE_EQ(result[1], (data[3] + data[4] + data[5]) / channels);
}

TEST(ChannelConverterTest, EmptyData) {
    std::vector<double> data;
    int channels = 2;
    ChannelConverter converter(channels, data);
    std::vector<double> result = converter.process();

    EXPECT_TRUE(result.empty());
}