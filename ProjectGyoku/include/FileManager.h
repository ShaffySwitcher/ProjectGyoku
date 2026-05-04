#pragma once

#include <string>
#include <map>

#ifdef DEBUG
#define USE_DATA_FOLDER
#endif
#include <cstdint>
#include <memory>

struct FileBuffer {
	FileBuffer() {};
	~FileBuffer();

	void* data = nullptr;
	size_t size = 0;
	size_t offset = 0;
	std::string path;

	template<typename T>
	T read() {
		T value{};
		readBytes(&value, sizeof(T));
		return value;
	}

	std::string readStr() {
		std::string result;
		char c;
		while ((c = read<char>()) != '\0') {
			result += c;
		}
		return result;
	}

	void readBytes(void* buffer, size_t size);
	void seek(size_t offset) { this->offset = offset; };
	size_t tell() const { return offset; };
};

struct SHF1Offset {
	uint64_t nameOffset;
	uint64_t nameSize;
	uint64_t dataOffset;
	uint64_t dataSize;
};

struct SHF1Header {
	char magic[4]; // "SHF1"
	uint16_t version;
	uint16_t padding;
	uint32_t fileCount;
};

struct SHF1File {
	uint64_t offset;
	uint64_t size;
};

struct LoadedSHF1 {
	std::shared_ptr<FileBuffer> file;
	std::map<const char*, SHF1File> files;
};

class FileManager {
public:
	static std::shared_ptr<FileBuffer> loadFile(const std::string& path, bool external = false);
	static bool load(std::string path);

private:
	static LoadedSHF1* archive;
};