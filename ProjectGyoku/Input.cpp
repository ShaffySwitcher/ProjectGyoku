#include "Input.h"
#include "Supervisor.h"
#include "Log.h"

#include <cstring>

bool Input::inputBuffer[256];
bool Input::inputBufferLastFrame[256];
bool Input::inputCurrent[256];
bool Input::inputPressed[256];
bool Input::inputReleased[256];

bool Input::gameInputCurrent[static_cast<uint8_t>(GameInput::GAME_INPUT_COUNT)];
bool Input::gameInputPressed[static_cast<uint8_t>(GameInput::GAME_INPUT_COUNT)];
bool Input::gameInputPressedRepeat[static_cast<uint8_t>(GameInput::LAST_KEY)];
bool Input::gameInputReleased[static_cast<uint8_t>(GameInput::GAME_INPUT_COUNT)];
bool Input::gameInputRawCurrent[static_cast<uint8_t>(GameInput::LAST_KEY)];
bool Input::gameInputRawCurrentLastFrame[static_cast<uint8_t>(GameInput::LAST_KEY)];
uint32_t Input::gameInputFrame[static_cast<uint8_t>(GameInput::LAST_KEY)];

void Input::update()
{
	memcpy_s(inputBufferLastFrame, sizeof(bool) * 256, inputBuffer, sizeof(bool) * 256);
	GetHitKeyStateAll((char*)inputBuffer);

	for (int i = 0; i < 256; i++) {
		inputCurrent[i] = inputBuffer[i];
		inputPressed[i] = (inputBuffer[i] && !inputBufferLastFrame[i]);
		inputReleased[i] = (!inputBuffer[i] && inputBufferLastFrame[i]);
	}

	int joypadInput = GetJoypadInputState(DX_INPUT_PAD1);
	memcpy_s(
		gameInputRawCurrentLastFrame,
		sizeof(bool) * static_cast<uint8_t>(GameInput::LAST_KEY),
		gameInputRawCurrent,
		sizeof(bool) * static_cast<uint8_t>(GameInput::LAST_KEY)
	);

	for(uint8_t i = 0; i < static_cast<uint8_t>(GameInput::LAST_KEY); i++) {
		const bool rawCurrent = inputCurrent[gSupervisor.config.keyboardMap[i]] || (joypadInput & gSupervisor.config.gamepadMap[i]);
		const bool rawLast = gameInputRawCurrentLastFrame[i];

		gameInputRawCurrent[i] = rawCurrent;
		gameInputCurrent[i] = rawCurrent;
		gameInputPressed[i] = rawCurrent && !rawLast;
		gameInputReleased[i] = !rawCurrent && rawLast;

		if (gameInputPressed[i]) {
			gameInputFrame[i] = gSupervisor.currentFrame;

		}
		if (rawCurrent) {
			gameInputPressedRepeat[i] = (gSupervisor.currentFrame - gameInputFrame[i]) % 8 == 0;
		}
		else {
			gameInputPressedRepeat[i] = false;
		}
	}

	// special edge cases for SELECT and CANCEL
	gameInputCurrent[static_cast<uint8_t>(GameInput::SELECT)] = gameInputCurrent[static_cast<uint8_t>(GameInput::FIRE)] || inputCurrent[KEY_INPUT_RETURN];
	gameInputPressed[static_cast<uint8_t>(GameInput::SELECT)] = gameInputPressed[static_cast<uint8_t>(GameInput::FIRE)] || inputPressed[KEY_INPUT_RETURN];
	gameInputReleased[static_cast<uint8_t>(GameInput::SELECT)] = gameInputReleased[static_cast<uint8_t>(GameInput::FIRE)] || inputReleased[KEY_INPUT_RETURN];

	gameInputCurrent[static_cast<uint8_t>(GameInput::CANCEL)] = gameInputCurrent[static_cast<uint8_t>(GameInput::BOMB)] || inputCurrent[KEY_INPUT_BACK];
	gameInputPressed[static_cast<uint8_t>(GameInput::CANCEL)] = gameInputPressed[static_cast<uint8_t>(GameInput::BOMB)] || inputPressed[KEY_INPUT_BACK];
	gameInputReleased[static_cast<uint8_t>(GameInput::CANCEL)] = gameInputReleased[static_cast<uint8_t>(GameInput::BOMB)] || inputReleased[KEY_INPUT_BACK];

}

void Input::clearGameInputState()
{
	memset(gameInputCurrent, 0, sizeof(bool) * static_cast<uint8_t>(GameInput::GAME_INPUT_COUNT));
	memset(gameInputPressed, 0, sizeof(bool) * static_cast<uint8_t>(GameInput::GAME_INPUT_COUNT));
	memset(gameInputReleased, 0, sizeof(bool) * static_cast<uint8_t>(GameInput::GAME_INPUT_COUNT));
	memset(gameInputPressedRepeat, 0, sizeof(bool) * static_cast<uint8_t>(GameInput::LAST_KEY));
}
