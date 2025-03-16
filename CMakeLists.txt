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

# Define LilyPond paths
set(LILYPOND_DIR "${CMAKE_SOURCE_DIR}/libs/lilypond/lilypond-2.24.4")
set(LILYPOND_EXE "${LILYPOND_DIR}/bin/lilypond.exe")
set(MUSICXML2LY "${LILYPOND_DIR}/bin/musicxml2ly.py")

# Ensure the executables exist
if(NOT EXISTS "${LILYPOND_EXE}")
    message(FATAL_ERROR "LilyPond executable not found at ${LILYPOND_EXE}")
endif()
if(NOT EXISTS "${MUSICXML2LY}")
    message(FATAL_ERROR "musicxml2ly.py not found at ${MUSICXML2LY}")
endif()

# Set up libmusicxml before using it
set(LIBMUSICXML_ROOT "${CMAKE_SOURCE_DIR}/libs/libmusicxml")

file(GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/src/backend/*.cpp"
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
