# CMakeList.txt : Top-level CMake project file, do global configuration
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
find_library(AUBIO_LIB aubio REQUIRED)
if (AUBIO_LIB)
    message(STATUS "Found aubio library at ${AUBIO_LIB}")
    target_link_libraries(ScoreGen PRIVATE ${AUBIO_LIB})
else()
    message(FATAL_ERROR "aubio library not found. Please ensure aubio is installed.")
endif()
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${AUBIO_INCLUDE_DIRS})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${AUBIO_LIBRARIES})

# Additional platform-specific settings
if (APPLE)
    target_link_libraries(ScoreGen PRIVATE "-framework CoreAudio" "-framework CoreFoundation")
endif()