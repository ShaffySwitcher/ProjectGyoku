#include "DxLib.h"

#include "Log.h"
#include "Supervisor.h"
#include <Windows.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include "Global.h"
#include "FileManager.h"
#include "Text.h"
#include "FPS.h"
#include "State.h"
#include "DebugScene.h"
#include "Texture.h"
#include "Sprite.h"
#include "resource.h"

void saveScreenshot();

bool checkForRunningGameInstance() {
	gSupervisor.exclusiveMutex = CreateMutex(NULL, TRUE, TEXT("Project Gyoku App"));

	if (gSupervisor.exclusiveMutex == NULL) {
		return false;
	}
	else if (GetLastError() == ERROR_ALREADY_EXISTS) {
		Log::error("Another instance of the game is running!");
	}

	return true;
}

bool requestWindow() {
	int bitDepth = (gSupervisor.config.flags & static_cast<uint8_t>(GameConfigFlags::USE_16_BIT_TEXTURES)) ? 16 : 32;
	int refreshRate = (gSupervisor.config.flags & static_cast<uint8_t>(GameConfigFlags::FORCE_REFRESH_RATE)) ? GetRefreshRate() : 60;
	float scale = (gSupervisor.config.flags & static_cast<uint8_t>(GameConfigFlags::FULLSCREEN)) ? 1.0f : gSupervisor.config.windowScale;

	if (SetGraphMode(static_cast<int>(WINDOW_WIDTH * scale), static_cast<int>(WINDOW_HEIGHT * scale), 32, refreshRate) != DX_CHANGESCREEN_OK) {
		Log::error("requestWindow(): SetGraphMode failed.");
		return false;
	}

	return true;
}

bool init() {
	Log::write("--- Initialization ---");
	
	FileManager::load("data.dat");

	if (SetGraphDisplayArea(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT) == -1) {
		Log::error("init(): SetGraphDisplayArea failed.");
		return false;
	}

	if (!requestWindow()) {
		return false;
	}

	if (ChangeWindowMode(!(gSupervisor.config.flags & static_cast<uint8_t>(GameConfigFlags::FULLSCREEN))) != DX_CHANGESCREEN_OK) {
		Log::write("init(): Could not obtain requested screen mode.");
	}

	if (SetAlwaysRunFlag(true) == -1) {
		Log::error("init(): SetAlwaysRunFlag failed.");
		return false;
	}

	if (SetWaitVSyncFlag(false) == -1) {
		Log::error("init(): SetWaitVSyncFlag failed.");
		return false;
	}

	if (SetBackgroundColor(20, 50, 30) == -1) {
		Log::error("init(): SetBackgroundColor failed.");
		return false;
	}

	if (SetWindowSizeChangeEnableFlag(false) == -1) {
		Log::error("init(): SetWindowSizeChangeEnableFlag failed.");
		return false;
	}

	if (SetOutApplicationLogValidFlag(false) == -1) {
		Log::error("init(): SetOutApplicationLogValidFlag failed.");
		return false;
	}

	if (SelectMidiMode(DX_MIDIMODE_DM) == -1) {
		Log::error("init(): SelectMidiMode failed.");
		return false;
	}

	if (SetFullScreenResolutionMode(DX_FSRESOLUTIONMODE_NATIVE) == -1) {
		Log::error("init(): SetFullScreenResolutionMode failed.");
		return false;
	}

#ifdef DEBUG
	if (SetMainWindowText("Project Gyoku ~ Sinful Reflections (v1.00a) (Debug Version)") == -1) {
#elif TRIAL
	if (SetMainWindowText("Project Gyoku ~ Sinful Reflections (v1.00a) (Trial Version)") == -1) {
#else
	if (SetMainWindowText("Project Gyoku ~ Sinful Reflections (v1.00a)") == -1) {
#endif
		Log::error("init(): SetMainWindowText failed.");
		return false;
	}

	if (SetMultiThreadFlag(true) == -1) {
		Log::error("init(): SetMultiThreadFlag failed.");
		return false;
	}

	if (SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8) == -1) {
		Log::error("init(): SetUseCharCodeFormat failed.");
		return false;
	}

	if (SetWindowIconID(IDI_ICON) == -1) {
		Log::error("init(): SetWindowIconHandle failed.");
		return false;
	}

	if (DxLib_Init() == -1) {
		Log::error("init(): DxLib failed to initialize.");
		return false;
	}

	Texture splashTexture;
	splashTexture.loadTexture("data/init.png");

	Background splashSurface;
	splashSurface.setTexture(&splashTexture);
	splashSurface.render();
	ScreenFlip();

#ifndef DEBUG
	WaitTimer(2*1000);
#endif

	if (SetTextureAddressMode(DX_TEXADDRESS_WRAP) == -1) {
		Log::error("init(): SetTextureAddressMode failed.");
		return false;
	}

	// Load native font
	if (!NativeText::init()) {
		Log::error("init(): NativeText failed to initialize.");
		return false;
	}

	// Load bitmap font
	if (!Text::init()) {
		Log::error("init(): Text failed to initialize.");
		return false;
	}

	FPS::init();

	Log::write("--- Initialization success! ---");

	return true;
}

bool loop() {
	if (ProcessMessage() != 0) { return false; }

	if (!(gSupervisor.config.flags & static_cast<uint16_t>(GameConfigFlags::VSYNC))) { FPS::wait(); }
	else { WaitVSync(1); }

	if (Input::inputCurrent[KEY_INPUT_LALT] && Input::inputPressed[KEY_INPUT_RETURN] || gSupervisor.wantFullscreen){
		gSupervisor.config.flags ^= static_cast<uint16_t>(GameConfigFlags::FULLSCREEN);

		requestWindow();
		ChangeWindowMode(!(gSupervisor.config.flags & static_cast<uint8_t>(GameConfigFlags::FULLSCREEN)));

		NativeText::restore();
		ANMManager::restore();
		gStateManager.restore();

		gSupervisor.wantFullscreen = false;
	}

	FPS::update();
	Input::update();

	return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
	if (!Log::init()) {
		return 1;
	}

	if (!checkForRunningGameInstance()) {
		Log::close();
		return 1;
	}

	gSupervisor.loadConfig("pg01.cfg");

	if (!init()) {
		Log::close();
		return 1;
	}

	gStateManager.setState(std::make_shared<DebugScene>());

    while (loop()) {
		if (!gStateManager.getState()) break;

		gStateManager.update();
		gStateManager.render();

		ScreenFlip();
		if (Input::inputPressed[KEY_INPUT_P]) saveScreenshot();
		gSupervisor.currentFrame++;
    }

	gSupervisor.saveConfig("pg01.cfg");
	DxLib_End();
	Log::close();

    return 0;
}

void saveScreenshot() {
	DWORD ftyp = GetFileAttributesA("screenshots");
	if (ftyp == INVALID_FILE_ATTRIBUTES) {
		if (!CreateDirectoryA("screenshots", NULL)) {
			Log::write("Failed to create \"screenshots\" directory!");
			return;
		}
	}

	std::string screenshotPath;
	int screenshotIndex = 0;

	while (true) {
		std::ostringstream oss;
		oss << "screenshots/pg_" << std::setw(3) << std::setfill('0') << screenshotIndex << ".bmp";
		screenshotPath = oss.str();

		std::ifstream screenshotFile(screenshotPath);
		if (!screenshotFile) {
			break;
		}

		screenshotFile.close();
		screenshotIndex++;
	}

	SaveDrawScreen(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, screenshotPath.c_str());
	Log::write("Screenshot saved to %s!", screenshotPath.c_str());
}