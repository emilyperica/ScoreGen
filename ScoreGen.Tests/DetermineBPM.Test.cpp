#include <gtest/gtest.h>
#include <vector>
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