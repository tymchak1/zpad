cmake_minimum_required(VERSION 3.10)
project(lab6_opencv)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

include_directories(include ${OpenCV_INCLUDE_DIRS})

add_executable(lab6
    src/main.cpp
    src/CameraProvider.cpp
    src/KeyProcessor.cpp
    src/FrameProcessor.cpp
    src/Display.cpp
)

target_link_libraries(lab6 ${OpenCV_LIBS})
