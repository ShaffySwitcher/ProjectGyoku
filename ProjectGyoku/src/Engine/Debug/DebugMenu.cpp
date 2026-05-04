#include "DebugMenu.h"
#include "DxLib.h"
#include "Input.h"
#include "Animation.h"
#include "Audio.h"
#include "FPS.h"
#include "Global.h"
#include "Profiler.h"
#include "State.h"
#include "Supervisor.h"
#include "Utils.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

bool DebugMenu::isOpen = false;
bool DebugMenu::isInteractable = false;
DebugMenu::Page DebugMenu::currentPage = DebugMenu::Page::ROOT;
std::vector<DebugMenu::Page> DebugMenu::pageStack{};
std::map<DebugMenu::Page, DebugMenu::MenuPageDefinition> DebugMenu::menuPages{};
int DebugMenu::selectedIndex = 0;
int DebugMenu::debugTargetFPS = 60;

int DebugMenu::menuScrollOffset = 0;

const char* DebugMenu::onOffLabel(bool value)
{
	return value ? "ON" : "OFF";
}

bool DebugMenu::isFineAdjustHeld()
{
	return Input::inputCurrent[KEY_INPUT_LSHIFT] || Input::inputCurrent[KEY_INPUT_RSHIFT];
}

void DebugMenu::syncMenuScrollWithSelection(int entryCount, int selected)
{
	const int maxOffset = (entryCount > MENU_MAX_VISIBLE_ENTRIES) ? (entryCount - MENU_MAX_VISIBLE_ENTRIES) : 0;
	menuScrollOffset = clamp(menuScrollOffset, 0, maxOffset);

	if (selected < menuScrollOffset) {
		menuScrollOffset = selected;
	}
	else if (selected >= menuScrollOffset + MENU_MAX_VISIBLE_ENTRIES) {
		menuScrollOffset = selected - MENU_MAX_VISIBLE_ENTRIES + 1;
	}

	menuScrollOffset = clamp(menuScrollOffset, 0, maxOffset);
}

bool DebugMenu::hasConfigFlag(GameConfigFlags flag)
{
	const uint16_t mask = static_cast<uint16_t>(flag);
	return (gSupervisor.config.flags & mask) != 0;
}

void DebugMenu::setConfigFlag(GameConfigFlags flag, bool enabled)
{
	const uint16_t mask = static_cast<uint16_t>(flag);

	if (enabled) {
		gSupervisor.config.flags = static_cast<uint16_t>(gSupervisor.config.flags | mask);
	}
	else {
		gSupervisor.config.flags = static_cast<uint16_t>(gSupervisor.config.flags & static_cast<uint16_t>(~mask));
	}
}

void DebugMenu::requestFullscreenState(bool shouldBeFullscreen)
{
	const bool fullscreenEnabled = hasConfigFlag(GameConfigFlags::FULLSCREEN);
	if (fullscreenEnabled != shouldBeFullscreen) {
		gSupervisor.wantFullscreen = true;
	}
}

void DebugMenu::requestWindowRecreate()
{
	gSupervisor.wantWindowRecreate = true;
}

const char* DebugMenu::getMusicModeLabel(uint8_t mode)
{
	switch (static_cast<GameConfigMusicMode>(mode)) {
	case GameConfigMusicMode::OFF:
		return "OFF";
	case GameConfigMusicMode::WAV:
		return "WAV";
	case GameConfigMusicMode::MIDI:
		return "MIDI";
	default:
		return "UNKNOWN";
	}
}

void DebugMenu::cycleMusicMode(int direction)
{
	int mode = clamp(static_cast<int>(gSupervisor.config.bgmType), static_cast<int>(GameConfigMusicMode::OFF), static_cast<int>(GameConfigMusicMode::MIDI));
	mode += (direction >= 0) ? 1 : -1;

	if (mode > static_cast<int>(GameConfigMusicMode::MIDI)) {
		mode = static_cast<int>(GameConfigMusicMode::OFF);
	}
	else if (mode < static_cast<int>(GameConfigMusicMode::OFF)) {
		mode = static_cast<int>(GameConfigMusicMode::MIDI);
	}

	gSupervisor.config.bgmType = static_cast<uint8_t>(mode);
}

const char* DebugMenu::getDifficultyLabel(uint8_t difficulty)
{
	switch (static_cast<Difficulty>(difficulty)) {
	case Difficulty::EASY:
		return "EASY";
	case Difficulty::NORMAL:
		return "NORMAL";
	case Difficulty::HARD:
		return "HARD";
	case Difficulty::ILLUSORY:
		return "ILLUSORY";
	case Difficulty::EXTRA:
		return "EXTRA";
	default:
		return "UNKNOWN";
	}
}

void DebugMenu::cycleDefaultDifficulty(int direction)
{
	int difficulty = clamp(static_cast<int>(gSupervisor.config.defaultDifficulty), static_cast<int>(Difficulty::EASY), static_cast<int>(Difficulty::ILLUSORY));
	difficulty += (direction >= 0) ? 1 : -1;

	if (difficulty > static_cast<int>(Difficulty::ILLUSORY)) {
		difficulty = static_cast<int>(Difficulty::EASY);
	}
	else if (difficulty < static_cast<int>(Difficulty::EASY)) {
		difficulty = static_cast<int>(Difficulty::ILLUSORY);
	}

	gSupervisor.config.defaultDifficulty = static_cast<uint8_t>(difficulty);
}

void DebugMenu::setSfxEnabled(bool enabled)
{
	gSupervisor.config.useSfx = enabled;
	
	if (!gSupervisor.config.useSfx) {
		SFXPlayer::stopAll();
		return;
	}
}

void DebugMenu::setOpen(bool open)
{
	DebugMenu::isOpen = open;
}
void DebugMenu::rebuildMenuModel()
{
	menuPages.clear();

	menuPages[Page::ROOT] = {
		"Debug Menu",
		{
			{ []() { return std::string("Runtime >"); }, []() { goToSubmenu(Page::RUNTIME); }, nullptr },
			{ []() { return std::string("Animation / ANM >"); }, []() { goToSubmenu(Page::ANIMATION); }, nullptr },
			{ []() { return std::string("Profiler >"); }, []() { goToSubmenu(Page::PROFILER); }, nullptr },
			{ []() { return std::string("Configuration >"); }, []() { goToSubmenu(Page::CONFIGURATION); }, nullptr },
			{ []() { return std::string("Close menu"); }, []() { setOpen(false); }, nullptr }
		}
	};

	menuPages[Page::RUNTIME] = {
		"Debug Menu / Runtime",
		{
			{
				[]() { return std::string("Target FPS: ") + std::to_string(debugTargetFPS); },
				nullptr,
				[](int direction) {
					const int step = isFineAdjustHeld() ? 1 : 5;
					debugTargetFPS = clamp(debugTargetFPS + (direction * step), 30, 240);
					FPS::setFPS(debugTargetFPS);
				}
			},
			{
				[]() { return format("Game Speed: %.2f", gGameManager.gameSpeed); },
				nullptr,
				[](int direction) {
					const float step = isFineAdjustHeld() ? 0.01f : 0.1f;
					gGameManager.gameSpeed = clamp(gGameManager.gameSpeed + (direction * step), 0.1f, 4.0f);
				}
			},
			{ []() { return std::string("Reset Game Speed"); }, []() { gGameManager.gameSpeed = 1.0f; }, nullptr },
			{ []() { return std::string("Reset Global Frame Counter"); }, []() { gSupervisor.currentFrame = 0; }, nullptr },
			{ []() { return std::string("< Back"); }, []() { goBack(); }, nullptr }
		}
	};

	menuPages[Page::ANIMATION] = {
		"Debug Menu / Animation",
		{
			{ []() { return std::string("Reload scripts"); }, []() { ANMManager::reloadScriptsAndRestartRunners(); }, nullptr },
			{ []() { return std::string("Reload scripts (in-place)"); }, []() { ANMManager::reloadScripts(); }, nullptr },
			{ []() { return std::string("Reload textures"); }, []() { ANMManager::reloadTextures(); }, nullptr },
			{ []() { return std::string("Unload all"); }, []() { ANMManager::unloadAll(); }, nullptr },
			{ []() { return std::string("< Back"); }, []() { goBack(); }, nullptr }
		}
	};

	menuPages[Page::PROFILER] = {
		"Debug Menu / Profiler",
		{
			{
				[]() { return std::string("Enabled: ") + (Profiler::isEnabled() ? "ON" : "OFF"); },
				[]() { Profiler::toggleEnabled(); },
				[](int direction) { Profiler::setEnabled(direction > 0); }
			},
			{
				[]() { return std::string("Overlay: ") + (Profiler::isOverlayEnabled() ? "ON" : "OFF"); },
				[]() { Profiler::toggleOverlayEnabled(); },
				[](int direction) { Profiler::setOverlayEnabled(direction > 0); }
			},
			{ []() { return format("Frame: %.2f ms", Profiler::getLastFrameMilliseconds()); }, nullptr, nullptr },
			{ []() { return format("Loop: %.2f ms", Profiler::getLastSampleMilliseconds("Loop")); }, nullptr, nullptr },
			{ []() { return format("State Update: %.2f ms", Profiler::getLastSampleMilliseconds("State Update")); }, nullptr, nullptr },
			{ []() { return format("State Render: %.2f ms", Profiler::getLastSampleMilliseconds("State Render")); }, nullptr, nullptr },
			{ []() { return std::string("< Back"); }, []() { goBack(); }, nullptr }
		}
	};

	menuPages[Page::CONFIGURATION] = {
		"Debug Menu / Configuration",
		{
			{ []() { return std::string("Video >"); }, []() { goToSubmenu(Page::CONFIGURATION_VIDEO); }, nullptr },
			{ []() { return std::string("Audio >"); }, []() { goToSubmenu(Page::CONFIGURATION_AUDIO); }, nullptr },
			{ []() { return std::string("Gameplay >"); }, []() { goToSubmenu(Page::CONFIGURATION_GAMEPLAY); }, nullptr },
			{
				[]() { return std::string("Reload configuration from file"); },
				[]() {
					gSupervisor.loadConfig("pg01.cfg");
				},
				nullptr
			},
			{ []() { return std::string("Save current configuration to file"); }, []() { gSupervisor.saveConfig("pg01.cfg"); }, nullptr },
			{
				[]() { return std::string("Reset to default configuration"); },
				[]() {
					gSupervisor.setDefaultConfig();
				},
				nullptr
			},
			{ []() { return std::string("< Back"); }, []() { goBack(); }, nullptr }
		}
	};

	menuPages[Page::CONFIGURATION_VIDEO] = {
		"Debug Menu / Configuration / Video",
		{
			{
				[]() {
					return std::string("Fullscreen: ") + onOffLabel(hasConfigFlag(GameConfigFlags::FULLSCREEN));
				},
				[]() { requestFullscreenState(!hasConfigFlag(GameConfigFlags::FULLSCREEN)); },
				[](int direction) { requestFullscreenState(direction > 0); }
			},
			{
				[]() {
					return std::string("VSync: ") + onOffLabel(hasConfigFlag(GameConfigFlags::VSYNC));
				},
				[]() { setConfigFlag(GameConfigFlags::VSYNC, !hasConfigFlag(GameConfigFlags::VSYNC)); },
				[](int direction) { setConfigFlag(GameConfigFlags::VSYNC, direction > 0); }
			},
			{
				[]() {
					return std::string("Force Refresh Rate: ") + onOffLabel(hasConfigFlag(GameConfigFlags::FORCE_REFRESH_RATE));
				},
				[]() { setConfigFlag(GameConfigFlags::FORCE_REFRESH_RATE, !hasConfigFlag(GameConfigFlags::FORCE_REFRESH_RATE)); },
				[](int direction) { setConfigFlag(GameConfigFlags::FORCE_REFRESH_RATE, direction > 0); }
			},
			{
				[]() {
					return std::string("16-bit Textures: ") + onOffLabel(hasConfigFlag(GameConfigFlags::USE_16_BIT_TEXTURES));
				},
				[]() { setConfigFlag(GameConfigFlags::USE_16_BIT_TEXTURES, !hasConfigFlag(GameConfigFlags::USE_16_BIT_TEXTURES)); },
				[](int direction) { setConfigFlag(GameConfigFlags::USE_16_BIT_TEXTURES, direction > 0); }
			},
			{
				[]() {
					return std::string("Disable Fog: ") + onOffLabel(hasConfigFlag(GameConfigFlags::DISABLE_FOG));
				},
				[]() { setConfigFlag(GameConfigFlags::DISABLE_FOG, !hasConfigFlag(GameConfigFlags::DISABLE_FOG)); },
				[](int direction) { setConfigFlag(GameConfigFlags::DISABLE_FOG, direction > 0); }
			},
			{
				[]() { return format("Window Scale: %.2f", gSupervisor.config.windowScale); },
				nullptr,
				[](int direction) {
					const float step = isFineAdjustHeld() ? 0.01f : 0.1f;
					gSupervisor.config.windowScale = clamp(gSupervisor.config.windowScale + (direction * step), 0.5f, 4.0f);
				}
			},
			{ []() { return std::string("Recreate Window Now"); }, []() { requestWindowRecreate(); }, nullptr },
			{ []() { return std::string("< Back"); }, []() { goBack(); }, nullptr }
		}
	};

	menuPages[Page::CONFIGURATION_AUDIO] = {
		"Debug Menu / Configuration / Audio",
		{
			{
				[]() {
					return std::string("BGM Mode: ") + getMusicModeLabel(gSupervisor.config.bgmType);
				},
				[]() { cycleMusicMode(1); },
				[](int direction) { cycleMusicMode(direction); }
			},
			{
				[]() { return std::string("BGM Volume: ") + std::to_string(static_cast<int>(gSupervisor.config.bgmVolume)); },
				nullptr,
				[](int direction) {
					int volume = static_cast<int>(gSupervisor.config.bgmVolume);
					const int step = isFineAdjustHeld() ? 1 : 5;
					volume = clamp(volume + (direction * step), 0, 100);
					gSupervisor.config.bgmVolume = static_cast<uint8_t>(volume);
				}
			},
			{
				[]() {
					return std::string("SFX Enabled: ") + onOffLabel(gSupervisor.config.useSfx);
				},
				[]() { setSfxEnabled(!gSupervisor.config.useSfx); },
				[](int direction) { setSfxEnabled(direction > 0); }
			},
			{
				[]() { return std::string("SFX Volume: ") + std::to_string(static_cast<int>(gSupervisor.config.sfxVolume)); },
				nullptr,
				[](int direction) {
					int volume = static_cast<int>(gSupervisor.config.sfxVolume);
					const int step = isFineAdjustHeld() ? 1 : 5;
					volume = clamp(volume + (direction * step), 0, 100);
					gSupervisor.config.sfxVolume = static_cast<uint8_t>(volume);
				}
			},
			{ []() { return std::string("< Back"); }, []() { goBack(); }, nullptr }
		}
	};

	menuPages[Page::CONFIGURATION_GAMEPLAY] = {
		"Debug Menu / Configuration / Gameplay",
		{
			{
				[]() {
					return std::string("Default Difficulty: ") + getDifficultyLabel(gSupervisor.config.defaultDifficulty);
				},
				[]() { cycleDefaultDifficulty(1); },
				[](int direction) { cycleDefaultDifficulty(direction); }
			},
			{
				[]() { return std::string("Frame Skip: ") + std::to_string(static_cast<int>(gSupervisor.config.frameSkip)); },
				nullptr,
				[](int direction) {
					int frameSkip = static_cast<int>(gSupervisor.config.frameSkip);
					frameSkip = clamp(frameSkip + direction, 0, 8);
					gSupervisor.config.frameSkip = static_cast<uint8_t>(frameSkip);
				}
			},
			{ []() { return std::string("< Back"); }, []() { goBack(); }, nullptr }
		}
	};
}

DebugMenu::MenuPageDefinition* DebugMenu::getPageDefinition(Page page)
{
	auto pageIt = menuPages.find(page);
	if (pageIt == menuPages.end()) {
		return nullptr;
	}

	return &pageIt->second;
}

int DebugMenu::getEntryCount(Page page)
{
	const MenuPageDefinition* pageDef = getPageDefinition(page);
	if (!pageDef) {
		return 0;
	}

	return static_cast<int>(pageDef->items.size());
}

void DebugMenu::resetSelection()
{
	selectedIndex = 0;
	menuScrollOffset = 0;
}

void DebugMenu::goToSubmenu(Page page)
{
	pageStack.push_back(currentPage);
	currentPage = page;
	resetSelection();
}

void DebugMenu::goBack()
{
	if (!pageStack.empty()) {
		currentPage = pageStack.back();
		pageStack.pop_back();
		resetSelection();
	}
	else {
		currentPage = Page::ROOT;
		resetSelection();
	}
}

void DebugMenu::adjustCurrentOption(int direction)
{
	MenuPageDefinition* pageDef = getPageDefinition(currentPage);
	if (!pageDef || selectedIndex < 0 || selectedIndex >= static_cast<int>(pageDef->items.size())) {
		return;
	}

	MenuItem& item = pageDef->items[selectedIndex];
	if (item.onAdjust) {
		item.onAdjust(direction);
	}
}

void DebugMenu::activateCurrentOption()
{
	MenuPageDefinition* pageDef = getPageDefinition(currentPage);
	if (!pageDef || selectedIndex < 0 || selectedIndex >= static_cast<int>(pageDef->items.size())) {
		return;
	}

	MenuItem& item = pageDef->items[selectedIndex];
	if (item.onActivate) {
		item.onActivate();
	}
}

std::string DebugMenu::getLine(Page page, int index)
{
	const MenuPageDefinition* pageDef = getPageDefinition(page);
	if (!pageDef || index < 0 || index >= static_cast<int>(pageDef->items.size())) {
		return "";
	}

	const MenuItem& item = pageDef->items[index];
	if (!item.label) {
		return "";
	}

	return item.label();
}

const char* DebugMenu::getPageTitle(Page page)
{
	const MenuPageDefinition* pageDef = getPageDefinition(page);
	if (!pageDef || !pageDef->title) {
		return "Debug Menu";
	}

	return pageDef->title;
}

void DebugMenu::init()
{
	DebugMenu::isOpen = false;
	DebugMenu::isInteractable = false;
	currentPage = Page::ROOT;
	pageStack.clear();
	menuPages.clear();
	rebuildMenuModel();
	selectedIndex = 0;
	menuScrollOffset = 0;
	debugTargetFPS = FPS::getFPS();
}

void DebugMenu::update()
{
	if (Input::inputPressed[KEY_INPUT_F8]) {
		DebugMenu::isInteractable = !DebugMenu::isInteractable;
		DebugMenu::isOpen = DebugMenu::isInteractable;

		if (DebugMenu::isInteractable) {
			currentPage = Page::ROOT;
			pageStack.clear();
			resetSelection();
			debugTargetFPS = FPS::getFPS();
		}
	}

	if (Input::inputPressed[KEY_INPUT_F7]) {
		DebugMenu::isOpen = !DebugMenu::isOpen;
	}

	if (!DebugMenu::isInteractable) {
		return;
	}

	if (!DebugMenu::isOpen) {
		Input::clearGameInputState();
		return;
	}

	rebuildMenuModel();

	const int entryCount = getEntryCount(currentPage);
	if (entryCount <= 0) {
		resetSelection();
		Input::clearGameInputState();
		return;
	}

	selectedIndex = clamp(selectedIndex, 0, entryCount - 1);

	const bool upPressed = Input::inputPressed[KEY_INPUT_UP];
	const bool downPressed = Input::inputPressed[KEY_INPUT_DOWN];
	const bool leftPressed = Input::inputPressed[KEY_INPUT_LEFT];
	const bool rightPressed = Input::inputPressed[KEY_INPUT_RIGHT];
	const bool activatePressed = Input::inputPressed[KEY_INPUT_RETURN] || Input::inputPressed[KEY_INPUT_Z];
	const bool cancelPressed = Input::inputPressed[KEY_INPUT_BACK] || Input::inputPressed[KEY_INPUT_X] || Input::inputPressed[KEY_INPUT_ESCAPE];

	if (upPressed) {
		selectedIndex--;
		if (selectedIndex < 0) {
			selectedIndex = entryCount - 1;
		}
	}

	if (downPressed) {
		selectedIndex++;
		if (selectedIndex >= entryCount) {
			selectedIndex = 0;
		}
	}

	if (leftPressed) {
		adjustCurrentOption(-1);
	}

	if (rightPressed) {
		adjustCurrentOption(1);
	}

	if (activatePressed) {
		activateCurrentOption();
	}

	if (cancelPressed) {
		goBack();
	}

	syncMenuScrollWithSelection(getEntryCount(currentPage), selectedIndex);

	Input::clearGameInputState();
}

void DebugMenu::render()
{
	if (DebugMenu::isOpen) {
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 204);
		DrawBox(140, 70, 620, 430, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		DrawBox(140, 70, 620, 430, GetColor(190, 190, 190), FALSE);
		DrawString(158, 86, getPageTitle(currentPage), GetColor(255, 255, 255));

		const int count = getEntryCount(currentPage);
		const int firstVisible = clamp(menuScrollOffset, 0, (count > MENU_MAX_VISIBLE_ENTRIES) ? (count - MENU_MAX_VISIBLE_ENTRIES) : 0);
		const int lastExclusive = (count < firstVisible + MENU_MAX_VISIBLE_ENTRIES) ? count : (firstVisible + MENU_MAX_VISIBLE_ENTRIES);

		for (int i = firstVisible; i < lastExclusive; i++) {
			const int y = MENU_FIRST_LINE_Y + ((i - firstVisible) * MENU_LINE_HEIGHT);
			const bool selected = DebugMenu::isInteractable && (i == selectedIndex);

			if (selected) {
				DrawBox(154, y - 2, 606, y + 18, GetColor(70, 70, 160), TRUE);
			}

			DrawString(160, y, getLine(currentPage, i).c_str(), GetColor(255, 255, 255));
		}

		if (count > MENU_MAX_VISIBLE_ENTRIES) {
			const int shownStart = firstVisible + 1;
			const int shownEnd = lastExclusive;
			DrawString(470, 86, format("%d-%d/%d", shownStart, shownEnd, count).c_str(), GetColor(200, 200, 200));

			if (firstVisible > 0) {
				DrawString(582, 104, "^", GetColor(220, 220, 220));
			}

			if (lastExclusive < count) {
				DrawString(582, 378, "v", GetColor(220, 220, 220));
			}
		}

		if (DebugMenu::isInteractable) {
			DrawString(158, 398, "Arrows: Move  Enter/Z: Select  Back/X/Esc: Back", GetColor(170, 170, 170));
		}
		else {
			DrawString(158, 398, "F8: enable interactive mode", GetColor(170, 170, 170));
		}
	}
}
