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

# Locate dependencies
find_package(Aubio CONFIG REQUIRED)
find_package(portaudio CONFIG REQUIRED)
find_package(SndFile REQUIRED)

# Link non-vcpkg dependencies
set(LIBMUSICXML_ROOT "${CMAKE_SOURCE_DIR}/libs/libmusicxml")
if(APPLE)
    set(LIBMUSICXML_LIBRARY "${LIBMUSICXML_ROOT}/macos/lib/libmusicxml2.a")
else()
    #
endif()

# Create the project's executable
file(GLOB SOURCES "src/*.cpp")
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

# Include headers
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE 
    ${CMAKE_HOME_DIRECTORY}/include
    ${LIBMUSICXML_ROOT}/include
)

# Link imported targets
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE 
    Aubio::aubio
    SndFile::sndfile
    portaudio
    ${LIBMUSICXML_LIBRARY}
)

# Additional platform-specific settings
if (APPLE)
    target_link_libraries(ScoreGen PRIVATE "-framework CoreAudio" "-framework CoreFoundation")
endif()
