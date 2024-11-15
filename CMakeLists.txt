cmake_minimum_required(VERSION 3.15)

set(VCPKG_DIR $ENV{VCPKG_DIR})
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake")

if(MSVC)
    set(VCPKG_CRT_LINKAGE static)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
project("ScoreGen")

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Add source to this project's executable.
file(GLOB SOURCES "src/*.cpp")
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

# Include headers
find_path(DSP dsp)
find_path(FINDKEY findKey)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_HOME_DIRECTORY}/include")

# Link libraries
find_package(SndFile REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SndFile::sndfile)
find_package(FFTW3 REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE FFTW3::fftw3)
find_path(FFTWPP_INCLUDE_DIRS "fftwpp/Array.cc")
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${FFTWPP_INCLUDE_DIRS})

# Additional platform-specific settings
if (APPLE)
    target_link_libraries(ScoreGen PRIVATE "-framework CoreAudio" "-framework CoreFoundation")
endif()
