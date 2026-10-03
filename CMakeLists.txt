cmake_minimum_required(VERSION 3.20)

project(
    signal-codec
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------
# Main application
# ------------------------------------------------------------

add_executable(signal-codec
    apps/main.cpp
    src/linecode/NrzL.cpp
    src/linecode/NrzI.cpp
)

# ------------------------------------------------------------
# Header files
# ------------------------------------------------------------

target_include_directories(signal-codec
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)