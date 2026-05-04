#include <DxLib.h>

#include "Engine/Log.h"
#include "Engine/Supervisor.h"
#include <Windows.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include "Engine/Global.h"
#include "Engine/FileManager.h"
#include "Engine/Graphics/Text.h"
#include "Engine/Math/FPS.h"
#include "Engine/State.h"
#include "Scene/DebugScene.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/Sprite.h"
#include "resource.h"
#include "Engine/Debug/DebugMenu.h"
#include "Engine/Debug/Profiler.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Score.h"

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
	if (SetMainWindowText("Project Gyoku ~ Sinful Reflections (v0.01) (Debug Version)") == -1) {
#elif TRIAL
	if (SetMainWindowText("Project Gyoku ~ Sinful Reflections (v0.01) (Trial Version)") == -1) {
#else
	if (SetMainWindowText("Project Gyoku ~ Sinful Reflections (v0.01)") == -1) {
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

	SFXPlayer::init();

#ifdef DEBUG
	DebugMenu::init();
#endif

	FPS::init();

	ScoreManager::load("score.dat");

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

	if (gSupervisor.wantWindowRecreate) {
		if (!requestWindow()) {
			Log::error("loop(): requestWindow failed during window recreation.");
		}

		if (ChangeWindowMode(!(gSupervisor.config.flags & static_cast<uint8_t>(GameConfigFlags::FULLSCREEN))) != DX_CHANGESCREEN_OK) {
			Log::write("loop(): Could not reapply window mode during window recreation.");
		}

		NativeText::restore();
		ANMManager::restore();
		gStateManager.restore();

		gSupervisor.wantWindowRecreate = false;
	}

	FPS::update();
	Input::update();
	BGMPlayer::update();
	SFXPlayer::update();

#ifdef DEBUG
	DebugMenu::update();
#endif

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

	while (true) {
		Profiler::beginFrame();

		bool shouldContinue = false;
		{
			PROFILE_SCOPE("Loop");
			shouldContinue = loop();
		}

		if (!shouldContinue) {
			Profiler::endFrame();
			break;
		}

		if (!gStateManager.getState()) {
			Profiler::endFrame();
			break;
		}

		{
			PROFILE_SCOPE("State Update");
			gStateManager.update();
			ScoreManager::updatePlaytime(gSupervisor.isInGame);
		}

		{
			PROFILE_SCOPE("State Render");
			gStateManager.render();
		}

#ifdef DEBUG
		{
			PROFILE_SCOPE("Debug Menu Render");
			DebugMenu::render();
		}
#endif

		Profiler::renderOverlay();

		{
			PROFILE_SCOPE("Screen Flip");
			ScreenFlip();
		}

		if (Input::inputPressed[KEY_INPUT_P]) saveScreenshot();
		gSupervisor.currentFrame++;

		Profiler::endFrame();
	}

	ScoreManager::save("score.dat");
	
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