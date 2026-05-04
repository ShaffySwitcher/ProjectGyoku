#include "FileManager.h"
#include <malloc.h>
#include "Log.h"
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <algorithm>
#include <vector>
#include <zlib.h>
#include <string.h>
#include <zconf.h>

LoadedSHF1* FileManager::archive = nullptr;

std::vector<unsigned char> decompressBuffer(const std::vector<unsigned char>& compressed)
{
	std::vector<unsigned char> decompressed;
	z_stream stream{};
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = compressed.size();
	stream.next_in = const_cast<Bytef*>(compressed.data());

	if (inflateInit(&stream) != Z_OK)
		return decompressed;

	int ret;
	unsigned char out[4096]{};
	do {
		stream.avail_out = sizeof(out);
		stream.next_out = out;
		ret = inflate(&stream, Z_NO_FLUSH);
		if (decompressed.size() < stream.total_out)
			decompressed.insert(decompressed.end(), out, out + sizeof(out) - stream.avail_out);
	} while (ret == Z_OK);

	inflateEnd(&stream);
	return decompressed;
}


FileBuffer::~FileBuffer() {
	if (data) {
		free(data);
		data = nullptr;

		Log::print("FileBuffer::~FileBuffer(): freed buffer for file '%s'", path.c_str());
	}
}

void FileBuffer::readBytes(void* buffer, size_t size)
{
	if (offset + size > this->size) {
		Log::error("FileBuffer::readBytes(): Attempt to read beyond the end of file '%s' (offset: %zu, size: %zu, requested: %zu)", path.c_str(), offset, this->size, size);
		return;
	}

	memcpy(buffer, static_cast<uint8_t*>(data) + offset, size);
	offset += size;
}

std::shared_ptr<FileBuffer> FileManager::loadFile(const std::string& path, bool external)
{
	std::shared_ptr<FileBuffer> fileBuffer = std::make_shared<FileBuffer>();

	if (external || !archive) {
		fileBuffer->path = path;

		std::ifstream file(path, std::ios::binary);
		if (!file.is_open()) {
			Log::print("FileManager::loadFile(): Cannot open external file '%s' (does it exist?)", path.c_str());
			return nullptr;
		}

		file.seekg(0, std::ios::end);
		fileBuffer->size = static_cast<size_t>(file.tellg());
		file.seekg(0, std::ios::beg);

		fileBuffer->data = malloc(fileBuffer->size);
		if (!fileBuffer->data) {
			Log::error("FileManager::loadFile(): Memory allocation failed for file '%s' (size: %zu)", path.c_str(), fileBuffer->size);
			return nullptr;
		}

		file.read(static_cast<char*>(fileBuffer->data), fileBuffer->size);
		file.close();

		Log::print("FileManager::loadFile(): Loaded external file '%s' (size: %zu bytes)", path.c_str(), fileBuffer->size);

		return fileBuffer;
	}
	else {
		std::string actualPath = path;
		if (actualPath.find("data/") == 0) {
			actualPath.erase(0, 5);
		}

		std::replace(actualPath.begin(), actualPath.end(), '/', '\\');

		for(auto& file : archive->files) {
			if (strcmp(file.first, actualPath.c_str()) == 0) {
				uint64_t compressedSize = file.second.size;
				void* compressedData = malloc(static_cast<size_t>(compressedSize));

				if (!compressedData) {
					Log::error("FileManager::loadFile(): Memory allocation failed for file '%s' in SHF1 archive (size: %llu)", actualPath.c_str(), compressedSize);
					return nullptr;
				}

				uint8_t* archiveData = static_cast<uint8_t*>(archive->file->data) + file.second.offset;

				uint8_t key = 0x24;
				for (size_t i = 0; i < compressedSize; i++) {
					uint8_t encrypted = archiveData[i];
					key = ((key << 3) & 0xFF) | ((key >> 5) & 7);
					uint8_t clear = encrypted ^ key;
					key += clear;
					static_cast<uint8_t*>(compressedData)[i] = clear;
				}

				std::vector<uint8_t> compressedBuffer(static_cast<uint8_t*>(compressedData), static_cast<uint8_t*>(compressedData) + compressedSize);
				std::vector<uint8_t> decompressedBuffer = decompressBuffer(compressedBuffer);

				free(compressedData);

				fileBuffer->size = decompressedBuffer.size();
				fileBuffer->data = malloc(fileBuffer->size);
				if (!fileBuffer->data) {
					Log::error("FileManager::loadFile(): Memory allocation failed for decompressed file '%s' (size: %zu)", actualPath.c_str(), fileBuffer->size);
					return nullptr;
				}
				memcpy_s(fileBuffer->data, fileBuffer->size, decompressedBuffer.data(), fileBuffer->size);
				
				Log::print("FileManager::loadFile(): Loaded file '%s' from SHF1 archive (size: %zu bytes)", actualPath.c_str(), fileBuffer->size);
				return fileBuffer;
			}
		}

		Log::print("FileManager::loadFile(): File '%s' not found in SHF1 archive", actualPath.c_str());
		return nullptr;
	}
}

bool FileManager::load(std::string path)
{
	Log::print("FileManager::load(): Loading SHF1 archive from '%s'...", path.c_str());

	std::shared_ptr<FileBuffer> fileBuffer = loadFile(path, true);
	if (!fileBuffer) {
#ifdef DEBUG
		Log::write(
#else
		Log::error(
#endif
			"FileManager::load(): Failed to load SHF1 archive from '%s'!", path.c_str());
		return false;
	}

	if(FileManager::archive) {
		delete FileManager::archive;
		FileManager::archive = nullptr;
	}

	FileManager::archive = new LoadedSHF1();
	FileManager::archive->file = fileBuffer;

	SHF1Header header = fileBuffer->read<SHF1Header>();
	if (strncmp(header.magic, "SHF1", 4) != 0) {
		Log::error("FileManager::load(): Invalid SHF1 archive (bad magic)!");
		delete FileManager::archive;
		FileManager::archive = nullptr;
		return false;
	}

	size_t offset = FileManager::archive->file->tell();

	for(uint32_t i = 0; i < header.fileCount; i++) {
		SHF1Offset shf1Offset = FileManager::archive->file->read<SHF1Offset>();

		offset = FileManager::archive->file->tell();

		FileManager::archive->file->seek(static_cast<size_t>(shf1Offset.nameOffset));
		char* nameBuffer = static_cast<char*>(malloc(static_cast<size_t>(shf1Offset.nameSize + 1)));
		nameBuffer[shf1Offset.nameSize] = '\0';
		FileManager::archive->file->readBytes(nameBuffer, static_cast<size_t>(shf1Offset.nameSize));

		SHF1File file{};
		file.offset = shf1Offset.dataOffset;
		file.size = shf1Offset.dataSize;

		FileManager::archive->files[nameBuffer] = file;
		FileManager::archive->file->seek(offset);
	}

	Log::print("FileManager::load(): Loaded SHF1 archive from '%s' with %u files.", path.c_str(), header.fileCount);
	return true;
}
