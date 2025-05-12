#pragma once
#include <string>
#include "windows.h"
#include <gdiplus.h>

class Gdiplus::Bitmap;

class FileOutput
{
public:
	void log(std::string text);
	void screenshot(POINT a, POINT b);
	void screenshotBitmap(Gdiplus::Bitmap *bmp);
	void DeleteOldLogs(int days);
};
