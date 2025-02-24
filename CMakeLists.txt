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
find_package(FFTW3 REQUIRED)

# Create the project's executable
file(GLOB SOURCES "src/backend/*.cpp")
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

# Link non-vcpkg dependencies
set(LIBMUSICXML_ROOT "${CMAKE_SOURCE_DIR}/libs/libmusicxml")
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
    libmusicxml
    FFTW3::fftw3
)

# Additional platform-specific settings
if (APPLE)
    target_link_libraries(ScoreGen PRIVATE "-framework CoreAudio" "-framework CoreFoundation")
endif()
