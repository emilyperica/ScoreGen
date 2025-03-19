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

# LilyPond setup
set(LILYPOND_VERSION "2.24.4")

# First try to find system-wide LilyPond
find_program(SYSTEM_LILYPOND_EXE lilypond)
find_program(SYSTEM_MUSICXML2LY musicxml2ly.py)

if(SYSTEM_LILYPOND_EXE AND SYSTEM_MUSICXML2LY)
    # Use system-installed LilyPond
    message(STATUS "Using system LilyPond: ${SYSTEM_LILYPOND_EXE}")
    set(LILYPOND_EXE ${SYSTEM_LILYPOND_EXE})
    set(MUSICXML2LY ${SYSTEM_MUSICXML2LY})
    set(LILYPOND_PYTHON python)  # Assume system Python
else()
    # Download LilyPond binaries
    message(STATUS "Downloading LilyPond ${LILYPOND_VERSION}")
    
    if(WIN32)
        set(LILYPOND_URL "https://gitlab.com/lilypond/lilypond/-/releases/v${LILYPOND_VERSION}/downloads/lilypond-${LILYPOND_VERSION}-mingw-x86_64.zip")
        set(LILYPOND_BIN_DIR "${CMAKE_BINARY_DIR}/lilypond/src/lilypond/bin")
        set(LILYPOND_EXE "${LILYPOND_BIN_DIR}/lilypond.exe")
        set(MUSICXML2LY "${LILYPOND_BIN_DIR}/musicxml2ly.py")
        set(LILYPOND_PYTHON "${LILYPOND_BIN_DIR}/python.exe")
    elseif(APPLE)
        set(LILYPOND_URL "https://gitlab.com/lilypond/lilypond/-/releases/v${LILYPOND_VERSION}/downloads/lilypond-${LILYPOND_VERSION}-darwin-x86_64.tar.gz")
        set(LILYPOND_BIN_DIR "${CMAKE_BINARY_DIR}/lilypond/usr/local/bin")
        set(LILYPOND_EXE "${LILYPOND_BIN_DIR}/lilypond")
        set(MUSICXML2LY "${LILYPOND_BIN_DIR}/musicxml2ly.py")
        set(LILYPOND_PYTHON "${LILYPOND_BIN_DIR}/python3")
    else()  # Linux
        set(LILYPOND_URL "https://gitlab.com/lilypond/lilypond/-/releases/v${LILYPOND_VERSION}/downloads/lilypond-${LILYPOND_VERSION}-linux-x86_64.tar.gz")
        set(LILYPOND_BIN_DIR "${CMAKE_BINARY_DIR}/lilypond/usr/local/bin")
        set(LILYPOND_EXE "${LILYPOND_BIN_DIR}/lilypond")
        set(MUSICXML2LY "${LILYPOND_BIN_DIR}/musicxml2ly.py")
        set(LILYPOND_PYTHON "${LILYPOND_BIN_DIR}/python3")
    endif()



    
    include(ExternalProject)
    ExternalProject_Add(
        lilypond
        URL ${LILYPOND_URL}
        PREFIX ${CMAKE_BINARY_DIR}/lilypond
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        EXCLUDE_FROM_ALL TRUE
    )
    # Create header file with paths
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/lilypond_paths.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/lilypond_paths.h
    )
endif()

# Set up libmusicxml ##########################################################
set(LIBMUSICXML_ROOT "${CMAKE_SOURCE_DIR}/libs/libmusicxml")

file(GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/src/backend/*.cpp"
)

# Create a library for the main code
add_library(ScoreGenLib STATIC ${SOURCES})

# Include headers for the library
target_include_directories(ScoreGenLib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${LIBMUSICXML_ROOT}/include
    ${CMAKE_CURRENT_BINARY_DIR}  # For generated lilypond_paths.h
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

if(NOT (SYSTEM_LILYPOND_EXE AND SYSTEM_MUSICXML2LY))
    # Ensure LilyPond is downloaded before building
    add_dependencies(ScoreGenLib lilypond)
endif()

# Rest of libmusicxml setup ###################################################
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