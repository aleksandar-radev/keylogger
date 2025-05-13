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
#include <filesystem>
// Visual Studio shortcut for adding library:
#pragma comment(lib, "Gdiplus.lib")

int LastSeconds = 0;

int FileOutput::lastDeletedImages = 0;
int FileOutput::lastDeletedLogs = 0;

void FileOutput::log(std::string text)
{
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	int CurrentSeconds = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(duration).count());

	// Create logs directory if it doesn't exist
	std::filesystem::create_directory("logs");

	// Get current date for filename
	SYSTEMTIME LastTime;
	GetLocalTime(&LastTime);
	char dateName[32];
	sprintf_s(dateName, "%04d-%02d-%02d.txt", LastTime.wYear, LastTime.wMonth, LastTime.wDay);
	std::string logPath = std::string("logs/") + dateName;

	std::ofstream outfile;
	outfile.open(logPath, std::ios_base::app); // append instead of overwrite

	if (LastSeconds + 60 < CurrentSeconds)
	{
		LastSeconds = CurrentSeconds;
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
						   (second.length() == 1 ? "0" + second : second) + "--> ";
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
	int new_w = static_cast<int>(w / 1.25);
	int new_h = static_cast<int>(h / 1.25);

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
	GetEncoderClsid(L"image/jpeg", &clsid); // Use JPEG

	CreateDirectoryW(L"images", NULL);

	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);

	std::wstring jpgName = L"images\\" +
						   std::to_wstring(LocalTime.wDay) + L"-" +
						   std::to_wstring(LocalTime.wMonth) + L"-" +
						   std::to_wstring(LocalTime.wYear) + L"=" +
						   std::to_wstring(LocalTime.wHour) + L"_" +
						   std::to_wstring(LocalTime.wMinute) + L"_" +
						   std::to_wstring(LocalTime.wSecond) + L".jpg";

	// Set JPEG quality
	Gdiplus::EncoderParameters encoderParameters;
	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
	encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;
	ULONG quality = 85; // 0-100, higher is better quality
	encoderParameters.Parameter[0].Value = &quality;

	bitmap.Save(jpgName.c_str(), &clsid, &encoderParameters);

	SelectObject(hDc, old_obj);
	DeleteDC(hDc);
	ReleaseDC(HWND_DESKTOP, hScreen);
	DeleteObject(hBitmap);
}

void FileOutput::screenshotBitmap(Gdiplus::Bitmap *bmp)
{
	CLSID clsid;
	GetEncoderClsid(L"image/jpeg", &clsid); // Use JPEG

	CreateDirectoryW(L"images", NULL);

	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);

	std::wstring jpgName = L"images\\" +
						   std::to_wstring(LocalTime.wDay) + L"-" +
						   std::to_wstring(LocalTime.wMonth) + L"-" +
						   std::to_wstring(LocalTime.wYear) + L"=" +
						   std::to_wstring(LocalTime.wHour) + L"_" +
						   std::to_wstring(LocalTime.wMinute) + L"_" +
						   std::to_wstring(LocalTime.wSecond) + L".jpg";

	Gdiplus::EncoderParameters encoderParameters;
	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
	encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;
	ULONG quality = 85;
	encoderParameters.Parameter[0].Value = &quality;

	bmp->Save(jpgName.c_str(), &clsid, &encoderParameters);
}

int FileOutput::DeleteOldLogs(int days)
{
	int deletedCount = 0;
	std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	if (std::filesystem::exists("./logs") && std::filesystem::is_directory("./logs"))
	{
		for (const auto &entry : std::filesystem::directory_iterator("./logs"))
		{
			const auto modifiedTime = std::filesystem::last_write_time(entry);
			const auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
				modifiedTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
			const auto modifiedTime_t = std::chrono::system_clock::to_time_t(sctp);
			double ageInDays = difftime(currentTime, modifiedTime_t) / (60 * 60 * 24);
			if (ageInDays > static_cast<double>(days))
			{
				std::filesystem::remove(entry);
				deletedCount++;
			}
		}
	}
	lastDeletedLogs = deletedCount;
	return deletedCount;
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