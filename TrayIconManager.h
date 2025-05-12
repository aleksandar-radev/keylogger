#pragma once
#include <windows.h>

class TrayIconManager {
public:
    TrayIconManager(HWND hWnd);
    ~TrayIconManager();
    void Show(bool isScreenshotsOn);
    void Remove();
    void UpdateStatus(bool isScreenshotsOn);
    void MinimizeToTray();
    void RestoreFromTray();
    HMENU GetMenu() const;
private:
    HWND m_hWnd;
    NOTIFYICONDATA m_nid;
    HMENU m_hTrayMenu;
    HICON CreateStatusTrayIcon(bool isOn);
    void DestroyIconIfNeeded();
};
