#pragma once
#include "FileOutput.h"
#include <iostream>
#include <chrono>
#include <format>
#include <fstream>
#include "windows.h"
#include "ShellAPI.h"
#include <string>
#include "gdiplus.h"
// Visual Studio shortcut for adding library:
#pragma comment(lib, "Gdiplus.lib")

int LastSeconds = 0;

void FileOutput::log(std::string text)
{
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	int CurrentSeconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

	std::ofstream outfile;
	outfile.open("file.txt", std::ios_base::app); // append instead of overwrite

	if (LastSeconds + 60 < CurrentSeconds)
	{
		LastSeconds = CurrentSeconds;
		SYSTEMTIME LastTime;
		GetLocalTime(&LastTime);
		std::string day = std::to_string(LastTime.wDay);
		std::string month = std::to_string(LastTime.wMonth);
		std::string year = std::to_string(LastTime.wYear);

		std::string hour = std::to_string(LastTime.wHour);
		std::string minute = std::to_string(LastTime.wMinute);
		std::string second = std::to_string(LastTime.wSecond);

		std::string date = "\n" +
						   (day.length() == 1 ? "0" + day : day) + "-" +
						   (month.length() == 1 ? "0" + month : month) + "-" +
						   year + " " +
						   (hour.length() == 1 ? "0" + hour : hour) + ":" +
						   (minute.length() == 1 ? "0" + minute : minute) + ":" +
						   (second.length() == 1 ? "0" + second : second) + "-->";
		outfile << date;
	}

	outfile << text;
	outfile.close();
}

int GetEncoderClsid(const WCHAR *format, CLSID *pClsid)
{
	UINT num = 0;  // number of image encoders
	UINT size = 0; // size of the image encoder array in bytes

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1; // Failure

	Gdiplus::ImageCodecInfo *pImageCodecInfo = (Gdiplus::ImageCodecInfo *)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1; // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j; // Success
		}
	}

	free(pImageCodecInfo);
	return -1; // Failure
}

void FileOutput::screenshot(POINT a, POINT b)
{
	int w = b.x - a.x;
	int h = b.y - a.y;
	int new_w = w / 1.25;
	int new_h = h / 1.25;

	if (w <= 0)
		return;
	if (h <= 0)
		return;

	std::cout << "taking screenshot\n";

	HDC hScreen = GetDC(HWND_DESKTOP);
	HDC hDc = CreateCompatibleDC(hScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, new_w, new_h);
	HGDIOBJ old_obj = SelectObject(hDc, hBitmap);
	SetStretchBltMode(hDc, HALFTONE);
	StretchBlt(hDc, 0, 0, new_w, new_h, hScreen, a.x, a.y, w, h, SRCCOPY);

	Gdiplus::Bitmap bitmap(hBitmap, NULL);

	CLSID clsid;

	GetEncoderClsid(L"image/png", &clsid);

	CreateDirectoryW(L"images", NULL);

	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);

	std::wstring pngName = L"images\\" +
						   std::to_wstring(LocalTime.wDay) + L"-" +
						   std::to_wstring(LocalTime.wMonth) + L"-" +
						   std::to_wstring(LocalTime.wYear) + L"=" +
						   std::to_wstring(LocalTime.wHour) + L"_" +
						   std::to_wstring(LocalTime.wMinute) + L"_" +
						   std::to_wstring(LocalTime.wSecond) + L".png";

	bitmap.Save(pngName.c_str(), &clsid);

	SelectObject(hDc, old_obj);
	DeleteDC(hDc);
	ReleaseDC(HWND_DESKTOP, hScreen);
	DeleteObject(hBitmap);
}

wchar_t *widen(const std::string &str)
{
	wchar_t *dest = new wchar_t[str.size() + 1];
	char *temp = new char[str.size()];
	for (int i = 0; i < str.size(); i++)
		dest[i] = str[i];
	dest[str.size()] = '\0';
	return dest;
}