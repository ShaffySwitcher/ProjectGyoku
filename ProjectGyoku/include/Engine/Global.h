#pragma once
#include <cstdint>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define GAME_WIDTH 384
#define GAME_HEIGHT 448
#define GAME_REGION_X 32
#define GAME_REGION_Y 16

#define ENGINE_VERSION 1

#define SPELLCARD_COUNT 64
#define STAGE_COUNT static_cast<uint8_t>(Stage::COUNT)
#define DIFFICULTY_COUNT static_cast<uint8_t>(Difficulty::COUNT)
#define SHOT_TYPE_COUNT static_cast<uint8_t>(ShotType::COUNT)
#define CHARACTER_COUNT static_cast<uint8_t>(Character::COUNT) * SHOT_TYPE_COUNT

#define TO_FRAMES(seconds) (static_cast<int>((seconds) * 60))
#define TO_SECONDS(frames) (static_cast<float>((frames)) / 60.0f)

enum class Difficulty : uint8_t {
	EASY,
	NORMAL,
	HARD,
	ILLUSORY,
	EXTRA,
	COUNT
};

enum class Stage : uint8_t {
	STAGE_1,
	STAGE_2,
	STAGE_3,
	STAGE_4,
	STAGE_5,
	STAGE_6,
	EXTRA,
	COUNT
};

enum class Character : uint8_t {
	LLOYD,
	COUNT
};

enum class ShotType : uint8_t {
	SHOT_TYPE_A,
	SHOT_TYPE_B,
	SHOT_TYPE_C,
	COUNT
};