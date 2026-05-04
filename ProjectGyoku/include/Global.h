#pragma once
#include <cstdint>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define GAME_WIDTH 384
#define GAME_HEIGHT 448
#define GAME_REGION_X 32
#define GAME_REGION_Y 16

#define ENGINE_VERSION 1

enum class Difficulty : uint8_t {
	EASY,
	NORMAL,
	HARD,
	ILLUSORY,
	EXTRA
};