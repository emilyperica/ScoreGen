cmake_minimum_required(VERSION 3.15)

# Set up vcpkg
set(VCPKG_DIR $ENV{VCPKG_DIR})
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake")

if(MSVC)
    set(VCPKG_CRT_LINKAGE static)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Define the test project
project(ScoreGen.Tests)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Locate dependencies
find_package(Aubio CONFIG REQUIRED)  # Find Aubio
find_package(portaudio CONFIG REQUIRED)
find_package(SndFile REQUIRED)
find_package(GTest REQUIRED)  # Add Google Test

# Add the main project's source files
file(GLOB MAIN_SOURCES "../src/*.cpp")  # Path to the main project's source files

# Add the test source files
file(GLOB TEST_SOURCES "tests/*.cpp")  # Path to the test source files

# Create the test executable
add_executable(${CMAKE_PROJECT_NAME} ${TEST_SOURCES} ${MAIN_SOURCES})

# Include the main project's headers
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/../include  # Path to the main project's include directory
)

# Link non-vcpkg dependencies
set(LIBMUSICXML_ROOT "${CMAKE_SOURCE_DIR}/../libs/libmusicxml")  # Adjusted path
if(APPLE)
    set(LIBMUSICXML_STATIC_PATH "${LIBMUSICXML_ROOT}/lib/macos/libmusicxml2.a")
    set(LIBMUSICXML_SHARED_PATH "${LIBMUSICXML_ROOT}/lib/macos/libmusicxml2.dylib")
    set(LIBMUSICXML_DEST "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/libmusicxml.dylib")
else()
    set(LIBMUSICXML_STATIC_PATH "${LIBMUSICXML_ROOT}/lib/win64/libmusicxml.lib")
    set(LIBMUSICXML_SHARED_PATH "${LIBMUSICXML_ROOT}/lib/win64/libmusicxml.dll")
    set(LIBMUSICXML_DEST "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/libmusicxml.dll")
endif()

add_library(libmusicxml SHARED IMPORTED)
set_target_properties(libmusicxml PROPERTIES
    IMPORTED_IMPLIB ${LIBMUSICXML_STATIC_PATH}
    IMPORTED_LOCATION ${LIBMUSICXML_SHARED_PATH}
)

# Copy libmusicxml library to same directory as executable
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${LIBMUSICXML_SHARED_PATH}
    ${LIBMUSICXML_DEST}
)

# Link imported targets
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    Aubio::aubio  # Link Aubio
    SndFile::sndfile
    portaudio
    libmusicxml
    GTest::GTest
    GTest::Main
)

# Additional platform-specific settings
if (APPLE)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE "-framework CoreAudio" "-framework CoreFoundation")
endif()