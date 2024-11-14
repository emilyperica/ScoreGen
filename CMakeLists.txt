<<<<<<< HEAD
﻿cmake_minimum_required(VERSION 3.15)

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
=======
﻿# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.8)
project("ScoreGen")

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Add source to this project's executable.
file(GLOB SOURCES "src/*.cpp")
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link libraries
find_package(SndFile REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SndFile::sndfile)
# Find FFTW include and library directories manually
find_path(FFTW3_INCLUDE_DIR fftw3.h PATHS /usr/local/include /usr/include)
find_library(FFTW3_LIBRARIES NAMES fftw3 PATHS /usr/local/lib /usr/lib)

if (FFTW3_INCLUDE_DIR AND FFTW3_LIBRARIES)
    message(STATUS "Found FFTW: ${FFTW3_LIBRARIES}")
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${FFTW3_INCLUDE_DIR})
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${FFTW3_LIBRARIES})
else()
    message(FATAL_ERROR "FFTW not found. Please ensure FFTW is installed.")
endif()
find_library(AUBIO_LIB aubio REQUIRED)
if (AUBIO_LIB)
    message(STATUS "Found aubio library at ${AUBIO_LIB}")
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${AUBIO_LIB})
else()
    message(FATAL_ERROR "aubio library not found. Please ensure aubio is installed.")
endif()
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${AUBIO_INCLUDE_DIRS})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${AUBIO_LIBRARIES})

<<<<<<< HEAD
# linking phase
target_link_directories(${CMAKE_PROJECT_NAME} PUBLIC ${includes})
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${linking_dependencies})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")
>>>>>>> 8a57175 (Issue #135: Adding cmake - WIP)
=======
# Additional platform-specific settings
if (APPLE)
    target_link_libraries(ScoreGen PRIVATE "-framework CoreAudio" "-framework CoreFoundation")
endif()
>>>>>>> 4796778 (Issue #135: modify CMakeLists.txt, add macos cmake presets)
