cmake_minimum_required(VERSION 3.10)
project(keylogger)

set(CMAKE_CXX_STANDARD 17)
set(SOURCES
    ghostly-log.cpp
    FileOutput.cpp
)

add_executable(keylogger ${SOURCES})

target_link_libraries(keylogger PRIVATE gdiplus)
