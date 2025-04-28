cmake_minimum_required(VERSION 3.10)

project(DigiviceSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find SDL2 libraries
find_package(SDL2 REQUIRED)
# Include SDL2 headers
include_directories(${SDL2_INCLUDE_DIRS})

# Define the executable name
set(EXECUTABLE_NAME DigiviceSim)

# Add source files for the executable
add_executable(${EXECUTABLE_NAME}
    src/main.cpp
    src/Game.cpp
    src/platform/pc/PCDisplay.cpp
    src/platform/pc/PCInput.cpp
)

# Add include directories (adjust paths if your structure differs slightly)
target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/assets # <<< Add path to your asset .h files
)

# Link executable against SDL2
target_link_libraries(${EXECUTABLE_NAME} ${SDL2_LIBRARIES})

# Optional: Add compiler flags (e.g., for warnings or optimization)
# target_compile_options(${EXECUTABLE_NAME} PRIVATE -Wall -Wextra -pedantic)
# target_compile_options(${EXECUTABLE_NAME} PRIVATE -O2)

# --- Installation (Optional) ---
# install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)

# --- Logging ---
message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "SDL2 Include Dirs: ${SDL2_INCLUDE_DIRS}")
message(STATUS "SDL2 Libraries: ${SDL2_LIBRARIES}")
message(STATUS "Source files added to ${EXECUTABLE_NAME}")