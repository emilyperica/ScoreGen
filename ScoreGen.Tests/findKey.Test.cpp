#include <gtest/gtest.h>
#include <numeric>
#include "findKey.h"

#define FP_TOL 1e-6

TEST(FindKeyTest, CorrelationOfIdentical) {
    std::vector<float> x = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::vector<int> y = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    float x_hat = std::accumulate(x.begin(), x.end(), 0.0) / x.size();
    float y_hat = std::accumulate(y.begin(), y.end(), 0.0) / y.size();
    float corr = getCorrelation(x_hat, y_hat, x, y);
    EXPECT_NEAR(corr, 1.0, FP_TOL);
}

TEST(FindKeyTest, CorrelationOfOpposite) {
    std::vector<float> x = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::vector<int> y = {12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    float x_hat = std::accumulate(x.begin(), x.end(), 0.0) / x.size();
    float y_hat = std::accumulate(y.begin(), y.end(), 0.0) / y.size();
    float corr = getCorrelation(x_hat, y_hat, x, y);
    EXPECT_NEAR(corr, -1.0, FP_TOL);
}

TEST(FindKeyTest, ExtractC_Major) {
    std::vector<int> durations = {6, 2, 3, 2, 4, 4, 2, 5, 2, 3, 2, 3};
    std::string key = findKey(durations);
    EXPECT_EQ(key, "C");
}

TEST(FindKeyTest, ExtractC_Minor) {
    std::vector<int> durations = {6, 2, 3, 5, 2, 3, 2, 4, 3, 2, 3, 3};
    std::string key = findKey(durations);
    EXPECT_EQ(key, "c");
}

TEST(FindKeyTest, ExtractA_MajorByCoefficient) {
    std::vector<int> durations(12, 1);
    durations[9] = 10;
    std::string key = findKey(durations);
    EXPECT_EQ(key, "A");
}

TEST(FindKeyTest, ExtractFSharp_MajorByRearrangeProfile) {
    std::vector<int> durations = {2, 5, 2, 3, 2, 3,6, 2, 3, 2, 4, 4 };
    std::string key = findKey(durations);
    EXPECT_EQ(key, "F#");
}

TEST(KeyFindingTest, KeyOfUniformDurations) {
    std::vector<int> durations(12, 1);
    EXPECT_EQ(findKey(durations), "C"); // i.e. the default key
}