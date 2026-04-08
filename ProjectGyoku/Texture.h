#pragma once

#include <DxLib.h>
#include <string>

class Texture
{
public:
	~Texture();

	bool loadTexture(const std::string& filePath, const std::string& alphaPath = "");
	void unload();

	int getWidth() { return width; }
	int getHeight() { return height; }
	int getHandle() { return handle; }

	bool restore();

private:
	int handle = -1;
	int width = -1;
	int height = -1;
	std::string path = "";
	std::string alphaPath = "";
};

