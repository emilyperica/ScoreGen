#include <gtest/gtest.h>
#include <iostream>

int main(int argc, char** argv) {
    // Your custom code
    std::cout << "Running tests...\n";

    // Initialize and run Google Test
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}