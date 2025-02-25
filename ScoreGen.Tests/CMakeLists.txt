set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add Google Test using FetchContent
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

file(GLOB TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)


# Create a test executable
add_executable(ScoreGen.Tests ${TEST_SOURCES})

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

add_test(NAME ScoreGen.Tests COMMAND ScoreGen.Tests)

message(STATUS "Setting test working directory to: ${CMAKE_SOURCE_DIR}/test/TestingDatasets")


set_tests_properties(ScoreGen.Tests PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test/TestingDatasets)

# Add the test to CTest
include(GoogleTest)
gtest_discover_tests(ScoreGen.Tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test/TestingDatasets
)