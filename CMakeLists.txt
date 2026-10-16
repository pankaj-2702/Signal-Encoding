cmake_minimum_required(VERSION 3.20)

project(
    signal-codec
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(signal-codec
    apps/main.cpp
    src/ui/Menu.cpp
    src/linecode/NrzL.cpp
    src/linecode/NrzI.cpp
    src/linecode/Manchester.cpp
    src/linecode/DiffManchester.cpp
    src/linecode/Ami.cpp
    src/generator/DataGenerator.cpp
    src/render/SignalPlotter.cpp
    src/ui/GeneratorPanel.cpp
    src/linecode/B8ZS.cpp
    src/linecode/HDB3.cpp
    
)

target_include_directories(signal-codec
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(signal-codec
    PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
)