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

#define APP_MUTEX_NAME L"KeyloggerAppMutex"
#define APP_WINDOW_CLASS L"KeyloggerMainDialogClass"

static KeyloggerCore keylogger;
static COLORREF logColor = RGB(0, 128, 0);
static COLORREF ssColor = RGB(0, 128, 0);
static std::wstring logText = L"Logging: OFF";
static std::wstring ssText = L"Screenshots: OFF";

// New: Button colors
static HBRUSH hLogBrush = NULL;
static HBRUSH hSSBrush = NULL;

// New: Tray icon and menu
static NOTIFYICONDATA nid = {0};
static HMENU hTrayMenu = NULL;

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void DeleteOldImages(int days);
void UpdateStatus(HWND hDlg);
void UpdateImagesSize(HWND hDlg);
void ShowTrayIcon(HWND hDlg);
void RemoveTrayIcon();
void MinimizeToTray(HWND hDlg);
void RestoreFromTray(HWND hDlg);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    HANDLE hMutex = CreateMutexW(NULL, FALSE, APP_MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // Already running, try to find and show the window
        HWND hOther = FindWindowW(APP_WINDOW_CLASS, NULL);
        if (hOther) {
            ShowWindow(hOther, SW_SHOW);
            SetForegroundWindow(hOther);
        }
        return 0;
    }
    // Register window class for FindWindow
    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
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
    // Update status labels (assume two static controls: IDC_LOG_STATUS_LABEL, IDC_SS_STATUS_LABEL)
    SetDlgItemText(hDlg, IDC_LOG_STATUS_LABEL, logText.c_str());
    SetDlgItemText(hDlg, IDC_SS_STATUS_LABEL, ssText.c_str());
    // Update button text with status in brackets
    std::wstring logBtn = log ? L"Stop Logging [ON]" : L"Start Logging [OFF]";
    std::wstring ssBtn = ss ? L"Stop Screenshots [ON]" : L"Start Screenshots [OFF]";
    SetDlgItemText(hDlg, IDC_LOG_BTN, logBtn.c_str());
    SetDlgItemText(hDlg, IDC_SCREENSHOT_BTN, ssBtn.c_str());
    // Update button brushes
    if (hLogBrush)
        DeleteObject(hLogBrush);
    if (hSSBrush)
        DeleteObject(hSSBrush);
    hLogBrush = CreateSolidBrush(logColor);
    hSSBrush = CreateSolidBrush(ssColor);
    UpdateImagesSize(hDlg);
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

void ShowTrayIcon(HWND hDlg) {
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hDlg;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcscpy_s(nid.szTip, L"Keylogger Running");
    Shell_NotifyIcon(NIM_ADD, &nid);
    if (!hTrayMenu) {
        hTrayMenu = CreatePopupMenu();
        AppendMenu(hTrayMenu, MF_STRING, ID_TRAY_EXIT, L"Quit");
    }
}

void RemoveTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid);
    if (hTrayMenu) {
        DestroyMenu(hTrayMenu);
        hTrayMenu = NULL;
    }
}

void MinimizeToTray(HWND hDlg) {
    ShowTrayIcon(hDlg);
    ShowWindow(hDlg, SW_HIDE);
}

void RestoreFromTray(HWND hDlg) {
    RemoveTrayIcon();
    ShowWindow(hDlg, SW_SHOW);
    SetForegroundWindow(hDlg);
}

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_INITDIALOG) {
        SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)hDlg);
        SetClassLongPtr(hDlg, GCLP_HBRBACKGROUND, (LONG_PTR)GetSysColorBrush(COLOR_3DFACE));
        SetWindowLongPtr(hDlg, GWLP_WNDPROC, (LONG_PTR)DefDlgProc);
        SetWindowLongPtr(hDlg, GWLP_ID, (LONG_PTR)APP_WINDOW_CLASS);
    }
    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemInt(hDlg, IDC_DAYS_EDIT, 7, FALSE);
        UpdateStatus(hDlg);
        keylogger.start();
        return TRUE;
    case WM_SYSCOMMAND:
        if ((wParam & 0xFFF0) == SC_CLOSE) {
            MinimizeToTray(hDlg);
            return TRUE;
        }
        break;
    case WM_TRAYICON:
        if (lParam == WM_LBUTTONDBLCLK) {
            RestoreFromTray(hDlg);
        } else if (lParam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hDlg);
            TrackPopupMenu(hTrayMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hDlg, NULL);
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_DELETE_BTN:
        {
            int days = GetDlgItemInt(hDlg, IDC_DAYS_EDIT, NULL, FALSE);
            DeleteOldImages(days);
            SetDlgItemText(hDlg, IDC_LOG_STATUS_LABEL, L"Old images deleted.");
            SetDlgItemText(hDlg, IDC_SS_STATUS_LABEL, L"");
            UpdateImagesSize(hDlg);
            break;
        }
        case IDC_DELETE_LOGS_BTN:
        {
            int days = GetDlgItemInt(hDlg, IDC_DAYS_EDIT, NULL, FALSE);
            FileOutput logger;
            logger.DeleteOldLogs(days);
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
        case ID_TRAY_EXIT:
            RemoveTrayIcon();
            keylogger.stop();
            if (hLogBrush) DeleteObject(hLogBrush);
            if (hSSBrush) DeleteObject(hSSBrush);
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
        MinimizeToTray(hDlg);
        return 0;
    }
    return FALSE;
}

void DeleteOldImages(int days)
{
    std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    if (std::filesystem::exists("./images") && std::filesystem::is_directory("./images"))
    {
        for (const auto &entry : std::filesystem::directory_iterator("./images"))
        {
            const auto modifiedTime = std::filesystem::last_write_time(entry);
            const auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                modifiedTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
            const auto modifiedTime_t = std::chrono::system_clock::to_time_t(sctp);
            double ageInDays = difftime(currentTime, modifiedTime_t) / (60 * 60 * 24);
            if (ageInDays > days)
            {
                std::filesystem::remove(entry);
            }
        }
    }
    // Also delete old logs
    FileOutput logger;
    logger.DeleteOldLogs(days);
}
