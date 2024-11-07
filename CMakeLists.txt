# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ScoreGen")
# Add source to this project's executable.
add_executable(${CMAKE_PROJECT_NAME} ${sources} "src/ScoreGen.cpp")

include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${includes})

# linking phase
target_link_directories(${CMAKE_PROJECT_NAME} PUBLIC ${includes})
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${linking_dependencies})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")
