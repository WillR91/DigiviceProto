# CMakeLists.txt - v4 (Using include_directories BEFORE add_executable)

cmake_minimum_required(VERSION 3.10)

# Project Name and Language
project(DigiviceSim LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Find Required Packages ---
find_package(SDL2 REQUIRED)

# --- Define Include Directories Globally (Alternative Approach) ---
# Add directories the compiler should search for headers
include_directories(
    # 1. Directory containing your project's headers (Game.h, IDisplay.h, etc.)
    ${CMAKE_SOURCE_DIR}/include

    # 2. Directory containing your asset headers (animation.h, etc.)
    #    ADJUST THIS PATH if your assets are not in 'assets/' at the project root
    ${CMAKE_SOURCE_DIR}/assets

    # 3. Directory containing SDL2 headers (variable provided by find_package)
    ${SDL2_INCLUDE_DIRS}
)

# --- Define Executable Target ---
set(EXECUTABLE_NAME DigiviceSim)

add_executable(${EXECUTABLE_NAME}
    # List all your .cpp source files here
    src/main.cpp
    src/Game.cpp
    src/platform/pc/PCDisplay.cpp
    src/platform/pc/PCInput.cpp
)

# --- Configure Target Properties ---

# Libraries to Link Against
target_link_libraries(${EXECUTABLE_NAME} PRIVATE
    # Link against the SDL2 library targets found by find_package
    ${SDL2_LIBRARIES}
)

# --- Logging ---
message(STATUS "-----------------------------------------------------")
message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "CMake Source Dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "Using SDL2 Include Dirs: ${SDL2_INCLUDE_DIRS}")
message(STATUS "Linking against SDL2 Libraries: ${SDL2_LIBRARIES}")
# Log the include directories CMake knows about *before* the target is defined
get_directory_property(dirs INCLUDE_DIRECTORIES)
message(STATUS "Include directories known by CMake: ${dirs}")
message(STATUS "-----------------------------------------------------")