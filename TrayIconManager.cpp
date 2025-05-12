#define UNICODE
#define _UNICODE
#include "TrayIconManager.h"
#include <shellapi.h>
#include "resource.h"

TrayIconManager::TrayIconManager(HWND hWnd) : m_hWnd(hWnd), m_hTrayMenu(NULL) {
    ZeroMemory(&m_nid, sizeof(m_nid));
    m_nid.cbSize = sizeof(NOTIFYICONDATA);
    m_nid.hWnd = hWnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;
}

TrayIconManager::~TrayIconManager() {
    Remove();
}

void TrayIconManager::Show(bool isScreenshotsOn) {
    DestroyIconIfNeeded();
    m_nid.hIcon = CreateStatusTrayIcon(isScreenshotsOn);
    wcscpy_s(m_nid.szTip, ARRAYSIZE(m_nid.szTip), L"Keylogger Running");
    Shell_NotifyIconW(NIM_ADD, &m_nid);
    if (m_hTrayMenu) {
        DestroyMenu(m_hTrayMenu);
        m_hTrayMenu = NULL;
    }
    m_hTrayMenu = CreatePopupMenu();
    if (isScreenshotsOn) {
        AppendMenuW(m_hTrayMenu, MF_STRING, ID_TRAY_SCREENSHOT_TOGGLE, L"Stop Screenshots");
    } else {
        AppendMenuW(m_hTrayMenu, MF_STRING, ID_TRAY_SCREENSHOT_TOGGLE, L"Start Screenshots");
    }
    AppendMenuW(m_hTrayMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(m_hTrayMenu, MF_STRING, ID_TRAY_EXIT, L"Quit");
}

void TrayIconManager::Remove() {
    Shell_NotifyIconW(NIM_DELETE, &m_nid);
    if (m_hTrayMenu) {
        DestroyMenu(m_hTrayMenu);
        m_hTrayMenu = NULL;
    }
    DestroyIconIfNeeded();
}

void TrayIconManager::UpdateStatus(bool isScreenshotsOn) {
    if (m_nid.hWnd && m_nid.uID) {
        DestroyIconIfNeeded();
        m_nid.hIcon = CreateStatusTrayIcon(isScreenshotsOn);
        Shell_NotifyIconW(NIM_MODIFY, &m_nid);
        if (m_hTrayMenu) {
            ModifyMenuW(m_hTrayMenu, ID_TRAY_SCREENSHOT_TOGGLE, MF_BYCOMMAND | MF_STRING,
                       ID_TRAY_SCREENSHOT_TOGGLE,
                       isScreenshotsOn ? L"Stop Screenshots" : L"Start Screenshots");
        }
    }
}

void TrayIconManager::MinimizeToTray() {
    ShowWindow(m_hWnd, SW_HIDE);
}

void TrayIconManager::RestoreFromTray() {
    Remove();
    ShowWindow(m_hWnd, SW_SHOW);
    SetForegroundWindow(m_hWnd);
}

HMENU TrayIconManager::GetMenu() const {
    return m_hTrayMenu;
}

void TrayIconManager::DestroyIconIfNeeded() {
    if (m_nid.hIcon) {
        DestroyIcon(m_nid.hIcon);
        m_nid.hIcon = NULL;
    }
}

HICON TrayIconManager::CreateStatusTrayIcon(bool isOn) {
    const int size = 32;
    HDC hdc = CreateCompatibleDC(NULL);
    BITMAPV5HEADER bi = {0};
    bi.bV5Size = sizeof(BITMAPV5HEADER);
    bi.bV5Width = size;
    bi.bV5Height = -size;
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask   = 0x00FF0000;
    bi.bV5GreenMask = 0x0000FF00;
    bi.bV5BlueMask  = 0x000000FF;
    bi.bV5AlphaMask = 0xFF000000;
    void* lpBits;
    HBITMAP hDib = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &lpBits, NULL, 0);
    HGDIOBJ oldDib = SelectObject(hdc, hDib);
    memset(lpBits, 0, size * size * 4);
    HBRUSH circle = CreateSolidBrush(isOn ? RGB(0, 200, 0) : RGB(200, 0, 0));
    int r = 12;
    int cx = size / 2, cy = size / 2;
    SelectObject(hdc, circle);
    Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
    DeleteObject(circle);
    ICONINFO ii = {0};
    ii.fIcon = TRUE;
    ii.hbmColor = hDib;
    ii.hbmMask = hDib;
    HICON hIcon = CreateIconIndirect(&ii);
    SelectObject(hdc, oldDib);
    DeleteObject(hDib);
    DeleteDC(hdc);
    return hIcon;
}
