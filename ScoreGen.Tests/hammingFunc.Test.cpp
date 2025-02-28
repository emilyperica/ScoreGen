#define FP_TOL 1e-6
#define _USE_MATH_DEFINES

#include <gtest/gtest.h>
#include "hammingFunction.h"
#include <cmath>



TEST(HammingFunctionTest, BroadWindowCheck) {
    int windowSize = 10;
    auto window = hammingFunction(windowSize);
    ASSERT_EQ(window.size(), windowSize);

    std::vector<double> expectedValues = {0.08, 0.187619556165, 
        0.460121838273, 0.77, 0.972258605561, 0.972258605561, 
        0.77, 0.460121838273, 0.187619556165, 0.08};

    for (int i = 0; i < expectedValues.size(); ++i) {
        EXPECT_NEAR(window[i], expectedValues[i], FP_TOL);
    }
}

TEST(HammingFunctionTest, WindowSize) {
    int windowSize = 5; 
    auto window = hammingFunction(windowSize);
    ASSERT_EQ(window.size(), windowSize);
}

TEST(HammingFunctionTest, WindowTaperValues) {
    int windowSize = 10; 
    auto window = hammingFunction(windowSize);

    double expectedStart = 0.54 - 0.46 * cos((2 * M_PI * 0) / (windowSize - 1));
    double expectedEnd = 0.54 - 0.46 * cos((2 * M_PI * 9) / (windowSize - 1));
    EXPECT_NEAR(window[0], expectedStart, FP_TOL);
    EXPECT_NEAR(window[windowSize-1], expectedEnd, FP_TOL);
}

TEST(HammingFunctionTest, EvenWindowSymmetry) {
    int windowSize = 10; 
    auto window = hammingFunction(windowSize);

    for (int i = 0; i < windowSize / 2; i++) {
        EXPECT_NEAR(window[i], window[windowSize - i - 1], FP_TOL);
    }
}

TEST(HammingFunctionTest, OddWindowSymmetry) {
    int windowSize = 11; 
    auto window = hammingFunction(windowSize);

    for (int i = 0; i < windowSize / 2; i++) {
        double l = window[i];
        double r = window[windowSize - i - 1];
        // Check that maximum value is in the middle
        EXPECT_GT(window[windowSize/2], l);
        EXPECT_GT(window[windowSize/2], r);
        EXPECT_NEAR(l, r, FP_TOL);
    }
}

TEST(HammingFunctionTest, UniqueWindow) {
    int windowSize = 10; 
    auto window1 = hammingFunction(windowSize);
    auto window2 = hammingFunction(windowSize);

    for (int i = 0; i < windowSize; i++) {
        EXPECT_NEAR(window1[i], window2[i], FP_TOL);
    }
}

TEST(HammingFunctionTest, InvalidWindowSize) {
    int windowSize = 0;
    EXPECT_THROW(hammingFunction(windowSize), std::invalid_argument);
}