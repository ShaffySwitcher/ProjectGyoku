#include "DebugMenu.h"
#include "DxLib.h"
#include "Input.h"
#include "Animation.h"
#include "FPS.h"
#include "Profiler.h"
#include "State.h"
#include "Supervisor.h"
#include "Utils.h"

#include <cstdint>
#include <string>
#include <vector>

bool DebugMenu::isOpen = false;
bool DebugMenu::isInteractable = false;
DebugMenu::Page DebugMenu::currentPage = DebugMenu::Page::ROOT;
std::vector<DebugMenu::Page> DebugMenu::pageStack{};
std::map<DebugMenu::Page, DebugMenu::MenuPageDefinition> DebugMenu::menuPages{};
int DebugMenu::selectedIndex = 0;
int DebugMenu::debugTargetFPS = 60;

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
			{ []() { return std::string("Graphics >"); }, []() { goToSubmenu(Page::GRAPHICS); }, nullptr },
			{ []() { return std::string("Animation / ANM >"); }, []() { goToSubmenu(Page::ANIMATION); }, nullptr },
			{ []() { return std::string("Profiler >"); }, []() { goToSubmenu(Page::PROFILER); }, nullptr },
			{ []() { return std::string("Close menu"); }, []() { setOpen(false); }, nullptr }
		}
	};

	menuPages[Page::RUNTIME] = {
		"Debug Menu / Runtime",
		{
			{
				[]() { return format("Game Speed: %.2f", gGameManager.gameSpeed); },
				nullptr,
				[](int direction) {
					gGameManager.gameSpeed = clamp(gGameManager.gameSpeed + (direction * 0.1f), 0.1f, 4.0f);
				}
			},
			{ []() { return std::string("Reset Game Speed"); }, []() { gGameManager.gameSpeed = 1.0f; }, nullptr },
			{ []() { return std::string("Reset Global Frame Counter"); }, []() { gSupervisor.currentFrame = 0; }, nullptr },
			{ []() { return std::string("< Back"); }, []() { goBack(); }, nullptr }
		}
	};

	menuPages[Page::GRAPHICS] = {
		"Debug Menu / Graphics",
		{
			{
				[]() { return std::string("Target FPS: ") + std::to_string(debugTargetFPS); },
				nullptr,
				[](int direction) {
					debugTargetFPS = clamp(debugTargetFPS + (direction * 5), 30, 240);
					FPS::setFPS(debugTargetFPS);
				}
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
			{
				[]() {
					return std::string("VSync: ") + ((gSupervisor.config.flags & static_cast<uint16_t>(GameConfigFlags::VSYNC)) ? "ON" : "OFF");
				},
				[]() { gSupervisor.config.flags ^= static_cast<uint16_t>(GameConfigFlags::VSYNC); },
				nullptr
			},
			{ []() { return std::string("Toggle Fullscreen"); }, []() { gSupervisor.wantFullscreen = true; }, nullptr },
			{ []() { return std::string("< Back"); }, []() { goBack(); }, nullptr }
		}
	};

	menuPages[Page::ANIMATION] = {
		"Debug Menu / Animation",
		{
			{ []() { return std::string("Reload scripts + restart runners"); }, []() { ANMManager::reloadScriptsAndRestartRunners(); }, nullptr },
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
		selectedIndex = 0;
	}
	else {
		currentPage = Page::ROOT;
		selectedIndex = 0;
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
			selectedIndex = 0;
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

	const int entryCount = getEntryCount(currentPage);
	if (entryCount <= 0) {
		selectedIndex = 0;
		Input::clearGameInputState();
		return;
	}

	if (selectedIndex >= entryCount) {
		selectedIndex = entryCount - 1;
	}

	if (Input::gameInputPressed[static_cast<uint8_t>(GameInput::UP)]) {
		selectedIndex--;
		if (selectedIndex < 0) {
			selectedIndex = entryCount - 1;
		}
	}

	if (Input::gameInputPressed[static_cast<uint8_t>(GameInput::DOWN)]) {
		selectedIndex++;
		if (selectedIndex >= entryCount) {
			selectedIndex = 0;
		}
	}

	if (Input::gameInputPressed[static_cast<uint8_t>(GameInput::LEFT)]) {
		adjustCurrentOption(-1);
	}

	if (Input::gameInputPressed[static_cast<uint8_t>(GameInput::RIGHT)]) {
		adjustCurrentOption(1);
	}

	if (Input::gameInputPressed[static_cast<uint8_t>(GameInput::SELECT)]) {
		activateCurrentOption();
	}

	if (Input::gameInputPressed[static_cast<uint8_t>(GameInput::CANCEL)]) {
		goBack();
	}

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
		for (int i = 0; i < count; i++) {
			const int y = 120 + (i * 24);
			const bool selected = DebugMenu::isInteractable && (i == selectedIndex);

			if (selected) {
				DrawBox(154, y - 2, 606, y + 18, GetColor(70, 70, 160), TRUE);
			}

			DrawString(160, y, getLine(currentPage, i).c_str(), GetColor(255, 255, 255));
		}

		if (DebugMenu::isInteractable) {
			DrawString(158, 398, format("Project Gyoku (Engine Version %i)", ENGINE_VERSION).c_str(), GetColor(170, 170, 170));
		}
		else {
			DrawString(158, 398, "F8: enable interactive mode", GetColor(170, 170, 170));
		}
	}
}
