#pragma once

#include <Windows.h>
#include <string>
#include <cstdint>
#include "Engine/Input.h"
#include "Engine/Global.h"
#include "Engine/Math/Random.h"

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
	bool isInGame = false;
	uint32_t currentFrame = 0;

	bool loadConfig(std::string path);
	bool saveConfig(std::string path);
	bool verifyConfig();
	void setDefaultConfig();
};

struct GameManager {
	GameManager() : rng(randomSeed) {}

	float gameSpeed = 1.0f;
	int gameSurface;
	int gameInterfaceSurface;
	uint32_t frame = 0;

	uint32_t shownScore = 0;
	uint32_t score = 0;
	uint32_t highscore = 0;

	uint8_t character = static_cast<uint8_t>(Character::LLOYD) * SHOT_TYPE_COUNT + static_cast<uint8_t>(ShotType::SHOT_TYPE_A);
	uint8_t difficulty = static_cast<uint8_t>(Difficulty::NORMAL);
	uint8_t stage = static_cast<uint8_t>(Stage::STAGE_1);
	
	uint8_t startingLives, startingBombs;
	int8_t lives;
	uint8_t deaths, continuesUsed;
	uint8_t bombs, bombsUsed;
	uint8_t power, powerBonus;
	uint16_t points, pointsTotal;
	uint16_t graze, grazeTotal;
	uint8_t spellcardsCaptured;
	
	bool inPracticeMode = false; // use PSCD highscore and quit to menu after stage clear
	bool inDemoMode = false; // play default replay and timeout after a while
	bool inReplayMode = false; // disable inputs and allow framerate change
	bool isTimeStopped = false; // self-explantory
	bool inRetryMenu = false; // X continues left, wanna retry?
	bool inPauseMenu = false; // pause menu active
	bool isGameCompleted = false; // full game clear
	uint32_t demoFrames = 0;

	uint16_t enemyNextItem = 0;		// this is the index of the next item to spawn
	uint16_t enemyDeathCount = 0;   // if this % 3 == 0, then spawn a random item
	
	uint32_t randomSeed = -1;
	RNG rng;
};

extern Supervisor gSupervisor;
extern GameManager gGameManager;