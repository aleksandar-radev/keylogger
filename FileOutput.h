#pragma once
#include <string>
#include "windows.h"

class FileOutput
{
public:
	void log(std::string text);
	void screenshot(POINT a, POINT b);
};

