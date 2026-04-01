#pragma once

#include <DxLib.h>
#include <cstdint>

enum class GameInput : uint8_t {
	LEFT,
	RIGHT,
	UP,
	DOWN,
	FIRE,
	BOMB,
	FOCUS,
	SPECIAL,
	PAUSE,
	SKIP,
	LAST_KEY,
	SELECT,		// maps to FIRE but also Enter
	CANCEL,		// maps to BOMB but also Return
	GAME_INPUT_COUNT
};

enum class GameInputReplay : uint8_t {
	LEFT,
	RIGHT,
	UP,
	DOWN,
	FIRE,
	BOMB,
	FOCUS,
	SPECIAL,
};

class Input
{
public:
	static bool inputCurrent[256];
	static bool inputPressed[256];
	static bool inputReleased[256];

	static bool gameInputCurrent[static_cast<uint8_t>(GameInput::GAME_INPUT_COUNT)];
	static bool gameInputPressed[static_cast<uint8_t>(GameInput::GAME_INPUT_COUNT)];
	static bool gameInputPressedRepeat[static_cast<uint8_t>(GameInput::LAST_KEY)];
	static bool gameInputReleased[static_cast<uint8_t>(GameInput::GAME_INPUT_COUNT)];

	static void update();
private:
	static bool inputBuffer[256];
	static bool inputBufferLastFrame[256];
	static bool gameInputCurrentLastFrame[static_cast<uint8_t>(GameInput::LAST_KEY)];
	static uint32_t gameInputFrame[static_cast<uint8_t>(GameInput::LAST_KEY)];
};

