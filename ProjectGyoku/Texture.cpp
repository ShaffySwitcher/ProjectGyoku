#include "Texture.h"
#include "Log.h"

Texture::~Texture()
{
	if (handle != -1)
	{
		DeleteGraph(handle);
	}
}

bool Texture::loadTexture(const std::string& filePath, const std::string& alphaPath)
{
	if (filePath.empty()) return false;

	std::shared_ptr<FileBuffer> textureFile = FileManager::loadFile(filePath);
	if (!textureFile){
		Log::error("Texture::loadTexture(): Failed to load texture file: %s", filePath.c_str());
		return false;
	}

	std::shared_ptr<FileBuffer> alphaFile = nullptr;
	if(!alphaPath.empty())
	{
		alphaFile = FileManager::loadFile(alphaPath);
		if (!alphaFile){
			Log::error("Texture::loadTexture(): Failed to load alpha texture file: %s", alphaPath.c_str());
			return false;
		}
	}

	handle = CreateGraphFromMem(textureFile->data, (int)textureFile->size, alphaFile ? (const void*)alphaFile->data : nullptr, alphaFile ? (int)alphaFile->size : 0);
	if (handle != -1) {
		GetGraphSize(handle, &width, &height);
	}
	else {
		Log::error("Texture::loadTexture(): CreateGraphFromMem() failed for file: %s", filePath.c_str());
		return false;
	}

	this->path = filePath;
	this->alphaPath = alphaPath;

	return true;
}

void Texture::restore()
{
	loadTexture(path, alphaPath);
}