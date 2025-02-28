#define _USE_MATH_DEFINES
#include <gtest/gtest.h>
#include <cmath>
#include "hanningFunction.h"

#define FP_TOL 1e-6

TEST(HanningFunctionTest, BroadWindowCheck) {
    int windowSize = 10;
    auto window = hanningFunction(windowSize);
    ASSERT_EQ(window.size(), windowSize);

    std::vector<double> expectedValues = {
        0.0, 0.11697777845, 0.413176, 0.75, 0.9698463, 0.9698463, 0.75, 0.413176, 0.11697778, 0.0
    };

    for (int i = 0; i < expectedValues.size(); i++) {
        EXPECT_NEAR(window[i], expectedValues[i], FP_TOL);
    }
}

TEST(HanningFunctionTest, WindowSize) {
    int windowSize = 5; 
    auto window = hanningFunction(windowSize);
    ASSERT_EQ(window.size(), windowSize);
}

TEST(HanningFunctionTest, WindowTaperValues) {
    int windowSize = 10;
    auto window = hanningFunction(windowSize);

    double expectedStart = 0.5 - 0.5 * cos((2 * M_PI * 0) / (windowSize - 1)); 
    double expectedEnd = 0.5 - 0.5 * cos((2 * M_PI * (windowSize - 1)) / (windowSize - 1));
    EXPECT_NEAR(window[0], expectedStart, FP_TOL);
    EXPECT_NEAR(window[windowSize - 1], expectedEnd, FP_TOL);
}

TEST(HanningFunctionTest, EvenWindowSymmetry) {
    int windowSize = 10;
    auto window = hanningFunction(windowSize);

    for (int i = 0; i < windowSize / 2; i++) {
        EXPECT_NEAR(window[i], window[windowSize - i - 1], FP_TOL);
    }
}

TEST(HanningFunctionTest, OddWindowSymmetry) {
    int windowSize = 11;
    auto window = hanningFunction(windowSize);

    for (int i = 0; i < windowSize / 2; i++) {
        double l = window[i];
        double r = window[windowSize - i - 1];

        EXPECT_GT(window[windowSize / 2], l);
        EXPECT_GT(window[windowSize / 2], r);
        EXPECT_NEAR(l, r, FP_TOL);
    }
}

TEST(HanningFunctionTest, UniqueWindow) {
    int windowSize = 10;
    auto window1 = hanningFunction(windowSize);
    auto window2 = hanningFunction(windowSize);

    for (int i = 0; i < windowSize; i++) {
        EXPECT_NEAR(window1[i], window2[i], FP_TOL);
    }
}

TEST(HanningFunctionTest, InvalidWindowSize) {
    int windowSize = 0;
    EXPECT_THROW(hanningFunction(windowSize), std::invalid_argument);
}
