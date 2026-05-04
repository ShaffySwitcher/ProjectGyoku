#include "Supervisor.h"
#include <cstdint>
#include <fstream>
#include <string>
#include "Global.h"
#include "Log.h"

Supervisor gSupervisor;
GameManager gGameManager;

bool Supervisor::loadConfig(std::string path)
{
	std::ifstream file(path, std::ios::binary);

	if (file) { file.read(reinterpret_cast<char*>(&gSupervisor.config), sizeof(GameConfiguration)); }
	else { setDefaultConfig(); }

	file.close();

	if (!verifyConfig()) {
		Log::write("Corrupted configuration, creating a new one...");
		setDefaultConfig();
	}

	return true;
}

bool Supervisor::saveConfig(std::string path)
{
	std::ofstream file(path, std::ios::binary);
	if (file.is_open()) {
		file.write(reinterpret_cast<char*>(&gSupervisor.config), sizeof(GameConfiguration));
	}
	else {
		Log::error("Cannot save configuration data!");
		return false;
	}

	file.close();

	return true;
}

bool Supervisor::verifyConfig()
{
	bool success = true;

	success &= (gSupervisor.config.frameSkip >= 0);
	success &= (
		gSupervisor.config.bgmType == static_cast<uint8_t>(GameConfigMusicMode::OFF) ||
		gSupervisor.config.bgmType == static_cast<uint8_t>(GameConfigMusicMode::WAV) ||
		gSupervisor.config.bgmType == static_cast<uint8_t>(GameConfigMusicMode::MIDI)
	);
	success &= (gSupervisor.config.bgmVolume >= 0 && gSupervisor.config.bgmVolume <= 100);
	success &= (gSupervisor.config.useSfx == true || gSupervisor.config.useSfx == false);
	success &= (gSupervisor.config.sfxVolume >= 0 && gSupervisor.config.sfxVolume <= 100);
	success &= (gSupervisor.config.defaultDifficulty >= static_cast<uint8_t>(Difficulty::EASY) && gSupervisor.config.defaultDifficulty <= static_cast<uint8_t>(Difficulty::ILLUSORY));
	success &= (gSupervisor.config.windowScale > 0.0);

	return success;
}

void Supervisor::setDefaultConfig()
{
	gSupervisor.config.flags = 0;
	gSupervisor.config.frameSkip = 0;
	gSupervisor.config.bgmType = static_cast<uint8_t>(GameConfigMusicMode::WAV);
	gSupervisor.config.bgmVolume = 100;
	gSupervisor.config.useSfx = true;
	gSupervisor.config.sfxVolume = 80;
	gSupervisor.config.defaultDifficulty = static_cast<uint8_t>(Difficulty::NORMAL);
	gSupervisor.config.windowScale = 1.0f;

	// default keyboard mapping
	gSupervisor.config.keyboardMap[static_cast<uint8_t>(GameInput::LEFT)] = KEY_INPUT_LEFT;
	gSupervisor.config.keyboardMap[static_cast<uint8_t>(GameInput::RIGHT)] = KEY_INPUT_RIGHT;
	gSupervisor.config.keyboardMap[static_cast<uint8_t>(GameInput::UP)] = KEY_INPUT_UP;
	gSupervisor.config.keyboardMap[static_cast<uint8_t>(GameInput::DOWN)] = KEY_INPUT_DOWN;
	gSupervisor.config.keyboardMap[static_cast<uint8_t>(GameInput::FIRE)] = KEY_INPUT_Z;
	gSupervisor.config.keyboardMap[static_cast<uint8_t>(GameInput::BOMB)] = KEY_INPUT_X;
	gSupervisor.config.keyboardMap[static_cast<uint8_t>(GameInput::FOCUS)] = KEY_INPUT_LSHIFT;
	gSupervisor.config.keyboardMap[static_cast<uint8_t>(GameInput::SPECIAL)] = KEY_INPUT_C;
	gSupervisor.config.keyboardMap[static_cast<uint8_t>(GameInput::PAUSE)] = KEY_INPUT_ESCAPE;
	gSupervisor.config.keyboardMap[static_cast<uint8_t>(GameInput::SKIP)] = KEY_INPUT_LCONTROL;

	// default gamepad mapping
	gSupervisor.config.gamepadMap[static_cast<uint8_t>(GameInput::LEFT)] = PAD_INPUT_LEFT;
	gSupervisor.config.gamepadMap[static_cast<uint8_t>(GameInput::RIGHT)] = PAD_INPUT_RIGHT;
	gSupervisor.config.gamepadMap[static_cast<uint8_t>(GameInput::UP)] = PAD_INPUT_UP;
	gSupervisor.config.gamepadMap[static_cast<uint8_t>(GameInput::DOWN)] = PAD_INPUT_DOWN;
	gSupervisor.config.gamepadMap[static_cast<uint8_t>(GameInput::FIRE)] = PAD_INPUT_2;
	gSupervisor.config.gamepadMap[static_cast<uint8_t>(GameInput::BOMB)] = PAD_INPUT_3;
	gSupervisor.config.gamepadMap[static_cast<uint8_t>(GameInput::FOCUS)] = PAD_INPUT_5;
	gSupervisor.config.gamepadMap[static_cast<uint8_t>(GameInput::SPECIAL)] = PAD_INPUT_1;
	gSupervisor.config.gamepadMap[static_cast<uint8_t>(GameInput::PAUSE)] = PAD_INPUT_10;
	gSupervisor.config.gamepadMap[static_cast<uint8_t>(GameInput::SKIP)] = PAD_INPUT_4;
}