#include "Input.h"
#include "Supervisor.h"
#include "Log.h"

bool Input::inputBuffer[256];
bool Input::inputBufferLastFrame[256];
bool Input::inputCurrent[256];
bool Input::inputPressed[256];
bool Input::inputReleased[256];

bool Input::gameInputCurrent[static_cast<uint8_t>(GameInput::GAME_INPUT_COUNT)];
bool Input::gameInputPressed[static_cast<uint8_t>(GameInput::GAME_INPUT_COUNT)];
bool Input::gameInputPressedRepeat[static_cast<uint8_t>(GameInput::LAST_KEY)];
bool Input::gameInputReleased[static_cast<uint8_t>(GameInput::GAME_INPUT_COUNT)];
bool Input::gameInputCurrentLastFrame[static_cast<uint8_t>(GameInput::LAST_KEY)];
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
	memcpy(&gameInputCurrentLastFrame, &gameInputCurrent, sizeof(bool) * static_cast<uint8_t>(GameInput::LAST_KEY));

	for(uint8_t i = 0; i < static_cast<uint8_t>(GameInput::LAST_KEY); i++) {
		gameInputCurrent[i] = inputCurrent[gSupervisor.config.keyboardMap[i]] || (joypadInput & gSupervisor.config.gamepadMap[i]);
		gameInputPressed[i] = gameInputCurrent[i] && !gameInputCurrentLastFrame[i];
		gameInputReleased[i] = !gameInputCurrent[i] && gameInputCurrentLastFrame[i];

		if (gameInputPressed[i]) {
			gameInputFrame[i] = gSupervisor.currentFrame;

		}
		if (gameInputCurrent[i]) {
			gameInputPressedRepeat[i] = (gSupervisor.currentFrame - gameInputFrame[i]) % 8 == 0;
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
