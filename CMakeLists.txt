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

add_compile_definitions(PA_ENABLE_DEBUG_OUTPUT=0)

# Add source to this project's executable.
file(GLOB SOURCES "src/*.cpp")
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

# Include headers
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_HOME_DIRECTORY}/include")

# Link libraries
set(LIBMUSICXML_ROOT "${CMAKE_SOURCE_DIR}/libs/libmusicxml")
if(APPLE)
    set(LIBMUSICXML_LIBRARY "${LIBMUSICXML_ROOT}/macos/lib/libmusicxml2.a")
else()
    #
endif()

find_package(SndFile REQUIRED)
find_package(FFTW3 REQUIRED)
find_package(portaudio CONFIG REQUIRED)
find_path(FFTWPP_INCLUDE_DIRS "fftwpp/Array.cc")

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SndFile::sndfile)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE FFTW3::fftw3)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE portaudio_static)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE "${LIBMUSICXML_LIBRARY}")

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${FFTWPP_INCLUDE_DIRS})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${LIBMUSICXML_ROOT}/include")

# Additional platform-specific settings
if (APPLE)
    target_link_libraries(ScoreGen PRIVATE "-framework CoreAudio" "-framework CoreFoundation")
endif()