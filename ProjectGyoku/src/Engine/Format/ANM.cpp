#include "Engine/Format/ANM.h"
#include "Engine/FileManager.h"
#include "Engine/Log.h"
#include "Engine/Global.h"
#include <unordered_map>

std::shared_ptr<ANM> ANM::load(const std::string& path, bool loadTexture)
{
	auto anm = std::make_shared<ANM>();
	anm->path = path;

	std::shared_ptr<FileBuffer> anmFile = FileManager::loadFile(path);
	if(!anmFile){
		Log::error("ANM::load(): Failed to load animation file: %s", path.c_str());
		return nullptr;
	}

	ANMHeader header;
	anmFile->readBytes(&header, sizeof(ANMHeader));

	if (strncmp(header.magic, "ANIM", 4) != 0) {
		Log::error("ANM::load(): Corrupt file: %s", path.c_str());
		return nullptr;
	}

	if (header.version != ENGINE_VERSION) {
		Log::error("ANM::load(): Unsupported ANM version %d in file: %s", header.version, path.c_str());
		return nullptr;
	}

	anmFile->seek(header.spriteTableOffset);
	for (uint32_t i = 0; i < header.numSprites; i++) {
		int32_t id = anmFile->read<int32_t>();
		Rect rectangle = anmFile->read<Rect>();

		anm->sprites[id] = rectangle;
	}

	anmFile->seek(header.scriptTableOffset);
	for (uint32_t i = 0; i < header.numScripts; i++) {
		int32_t id = anmFile->read<int32_t>();
		uint32_t scriptOffset = anmFile->read<uint32_t>();

		ANMScript& script = anm->scripts[id];

		size_t offset = anmFile->tell();

		anmFile->seek(scriptOffset);

		while (true) {
			ANMInstruction instruction;

			instruction.offset = anmFile->tell() - scriptOffset;
			instruction.time = anmFile->read<uint16_t>();
			instruction.opcode = static_cast<ANMOpcode>(anmFile->read<uint8_t>());

			uint8_t size = anmFile->read<uint8_t>();
			instruction.args.resize(size);
			anmFile->readBytes(instruction.args.data(), size);

			script.instructions.push_back(instruction);

			if (instruction.opcode == ANMOpcode::STOP) {
				break;
			}
		}

		std::unordered_map<uint32_t, uint32_t> offsetToIndex;

		for (uint32_t i = 0; i < script.instructions.size(); i++) {
			offsetToIndex[script.instructions[i].offset] = i;
		}

		for (uint32_t i = 0; i < script.instructions.size(); i++) {
			auto& instr = script.instructions[i];

			switch (instr.opcode) {
				case ANMOpcode::INTERRUPT_LABEL: {
					int32_t label = instr.get<int32_t>();
					script.interrupts[label] = i;
					break;
				}
			}
		}

		anmFile->seek(offset);
	}

	if (loadTexture && header.pathOffset) {
		std::string textureAlphaPath{};
		std::string texturePath{};

		if (header.pathOffsetAlpha) {
			anmFile->seek(header.pathOffsetAlpha);
			textureAlphaPath = anmFile->readStr();
		}

		
		anmFile->seek(header.pathOffset);
		texturePath = anmFile->readStr();

		anm->texture.loadTexture(texturePath.c_str(), header.pathOffsetAlpha ? textureAlphaPath.c_str() : "");
	}

	return anm;
}