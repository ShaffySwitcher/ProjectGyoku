#pragma once

#include <DxLib.h>
#include "FileManager.h"
#include "Math.h"

class Texture
{
public:
	~Texture();

	bool loadTexture(const std::string& filePath, const std::string& alphaPath = "");

	int getWidth() { return width; }
	int getHeight() { return height; }
	int getHandle() { return handle; }

	void restore();

private:
	int handle = -1;
	int width = -1;
	int height = -1;
	std::string path = "";
	std::string alphaPath = "";
};

