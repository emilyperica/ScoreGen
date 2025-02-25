cmake_minimum_required(VERSION 3.15)

# Set up vcpkg
set(VCPKG_DIR $ENV{VCPKG_DIR})
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake")

# MSVC-specific settings
if(MSVC)
    set(VCPKG_CRT_LINKAGE static)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Project name
project("ScoreGen")

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Locate dependencies
find_package(Aubio CONFIG REQUIRED)
find_package(portaudio CONFIG REQUIRED)
find_package(SndFile REQUIRED)
find_package(FFTW3 REQUIRED)

# Set up libmusicxml before using it
set(LIBMUSICXML_ROOT "${CMAKE_SOURCE_DIR}/libs/libmusicxml")

# Explicitly list all source files instead of using GLOB
set(SOURCES
    src/backend/channelConverter.cpp
    src/backend/detectNoteDuration.cpp
    src/backend/determineBPM.cpp
    src/backend/dsp.cpp
    src/backend/findKey.cpp
    src/backend/generateMusicXML.cpp
    src/backend/hammingFunction.cpp
    src/backend/recordAudio.cpp
    src/backend/ScoreGen.cpp
    src/backend/sndFileHandler.cpp
    src/backend/STFT.cpp
)

# Create a library for the main code
add_library(ScoreGenLib STATIC ${SOURCES})

# Include headers for the library
target_include_directories(ScoreGenLib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${LIBMUSICXML_ROOT}/include  # Make sure this is correctly set
)

# Link dependencies to the library
target_link_libraries(ScoreGenLib PUBLIC
    Aubio::aubio
    SndFile::sndfile
    portaudio
    libmusicxml
    FFTW3::fftw3
)

# Create the executable and link it to the library
add_executable(ScoreGen src/backend/ScoreGen.cpp)
target_link_libraries(ScoreGen PRIVATE ScoreGenLib)

# Set up libmusicxml
if(APPLE)
    set(LIBMUSICXML_STATIC_PATH "${LIBMUSICXML_ROOT}/lib/macos/libmusicxml2.a")
    set(LIBMUSICXML_SHARED_PATH "${LIBMUSICXML_ROOT}/lib/macos/libmusicxml2.dylib")
    set(LIBMUSICXML_DEST "$<TARGET_FILE_DIR:ScoreGen>/libmusicxml.dylib")
else()
    set(LIBMUSICXML_STATIC_PATH "${LIBMUSICXML_ROOT}/lib/win64/libmusicxml.lib")
    set(LIBMUSICXML_SHARED_PATH "${LIBMUSICXML_ROOT}/lib/win64/libmusicxml.dll")
    set(LIBMUSICXML_DEST "$<TARGET_FILE_DIR:ScoreGen>/libmusicxml.dll")
endif()

add_library(libmusicxml SHARED IMPORTED)
set_target_properties(libmusicxml PROPERTIES
    IMPORTED_IMPLIB ${LIBMUSICXML_STATIC_PATH}
    IMPORTED_LOCATION ${LIBMUSICXML_SHARED_PATH}
)

# Copy libmusicxml library to the same directory as the executable
add_custom_command(TARGET ScoreGen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${LIBMUSICXML_SHARED_PATH}
    ${LIBMUSICXML_DEST}
)

# Additional platform-specific settings
if (APPLE)
    target_link_libraries(ScoreGen PRIVATE "-framework CoreAudio" "-framework CoreFoundation")
endif()

# Enable testing and add the tests directory
enable_testing()
add_subdirectory(ScoreGen.Tests)
