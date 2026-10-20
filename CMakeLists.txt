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
    src/analog/Pcm.cpp
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

enable_testing()

add_executable(test_pcm tests/test_pcm.cpp src/analog/Pcm.cpp)
target_include_directories(test_pcm PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME test_pcm COMMAND test_pcm)

add_executable(test_generator tests/test_generator.cpp src/generator/DataGenerator.cpp)
target_include_directories(test_generator PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME test_generator COMMAND test_generator)

add_executable(test_scramblers tests/test_scramblers.cpp src/linecode/B8ZS.cpp src/linecode/HDB3.cpp src/linecode/Ami.cpp)
target_include_directories(test_scramblers PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME test_scramblers COMMAND test_scramblers)