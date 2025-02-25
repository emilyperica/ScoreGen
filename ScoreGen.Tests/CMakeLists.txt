# Add Google Test using FetchContent
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Create a test executable
add_executable(ScoreGen.Tests
    ScoreGen.Test.cpp
    DetermineBPM.Test.cpp
)

# Ensure ScoreGenLib propagates all necessary include directories
target_include_directories(ScoreGen.Tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include  # Main project headers
    ${LIBMUSICXML_ROOT}/include
)

# Link Google Test and ScoreGenLib
target_link_libraries(ScoreGen.Tests PRIVATE
    gtest
    gtest_main
    ScoreGenLib  # Link against the library
)

# Add the test to CTest
include(GoogleTest)
gtest_discover_tests(ScoreGen.Tests)
