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
//Visual Studio shortcut for adding library:
#pragma comment(lib, "Gdiplus.lib")

void FileOutput::log(std::string text)
{
	std::ofstream outfile;
	outfile.open("file.txt", std::ios_base::app); // append instead of overwrite
	outfile << text;
}





int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

void FileOutput::screenshot(POINT a, POINT b)
{
	int w = b.x - a.x;
	int h = b.y - a.y;

	if (w <= 0) return;
	if (h <= 0) return;


	std::cout << "taking screenshot\n";

	HDC     hScreen = GetDC(HWND_DESKTOP);
	HDC     hDc = CreateCompatibleDC(hScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, w, h);
	HGDIOBJ old_obj = SelectObject(hDc, hBitmap);
	BitBlt(hDc, 0, 0, w, h, hScreen, a.x, a.y, SRCCOPY);

	Gdiplus::Bitmap bitmap(hBitmap, NULL);
	CLSID clsid;

	GetEncoderClsid(L"image/png", &clsid);

	CreateDirectory(L"images", NULL);

	std::wstring pngName = std::format(L"images\\{:%d-%m-%Y=%H_%M_%OS}.png", std::chrono::system_clock::now());

	bitmap.Save(pngName.c_str(), &clsid);

	SelectObject(hDc, old_obj);
	DeleteDC(hDc);
	ReleaseDC(HWND_DESKTOP, hScreen);
	DeleteObject(hBitmap);
}

wchar_t* widen(const std::string& str) {
	wchar_t* dest = new wchar_t[str.size() + 1];
	char* temp = new char[str.size()];
	for (int i = 0; i < str.size(); i++)
		dest[i] = str[i];
	dest[str.size()] = '\0';
	return dest;
}