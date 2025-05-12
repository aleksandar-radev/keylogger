cmake_minimum_required(VERSION 3.10)
project(keylogger)

set(CMAKE_CXX_STANDARD 17)
set(SOURCES
    FileOutput.cpp
    KeyloggerCore.cpp
    winmain.cpp
    TrayIconManager.cpp
    CleanupManager.cpp
    keylogger.rc
)

add_executable(keylogger ${SOURCES})
set_target_properties(keylogger PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(keylogger PRIVATE gdiplus)
