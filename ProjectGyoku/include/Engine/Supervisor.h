#pragma once

#include <Windows.h>
#include <string>
#include <cstdint>
#include "Engine/Input.h"

enum class GameConfigMusicMode : uint8_t {
	OFF = 0,
	WAV = 1,
	MIDI = 2
};

enum class GameConfigFlags : uint8_t {
	FULLSCREEN = 1 << 0,
	VSYNC = 1 << 1,
	FORCE_REFRESH_RATE = 1 << 2,
	USE_16_BIT_TEXTURES = 1 << 3,
	DISABLE_FOG = 1 << 4,
};

struct GameConfiguration {
	uint16_t flags;
	uint8_t frameSkip;
	uint8_t bgmType;
	uint8_t bgmVolume;
	bool useSfx;
	uint8_t sfxVolume;
	uint8_t defaultDifficulty;
	float windowScale;
	uint8_t keyboardMap[static_cast<uint8_t>(GameInput::LAST_KEY)];
	uint32_t gamepadMap[static_cast<uint8_t>(GameInput::LAST_KEY)];
};

struct Supervisor
{
	HANDLE exclusiveMutex;
	GameConfiguration config; // configuration data
	
	bool wantFullscreen = false;
	bool wantWindowRecreate = false;
	uint32_t currentFrame = 0;

	bool loadConfig(std::string path);
	bool saveConfig(std::string path);
	bool verifyConfig();
	void setDefaultConfig();
};

struct GameManager {
	float gameSpeed = 1.0f;
};

extern Supervisor gSupervisor;
extern GameManager gGameManager;