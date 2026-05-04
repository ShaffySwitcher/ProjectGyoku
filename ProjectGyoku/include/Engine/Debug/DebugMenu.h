#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Engine/Audio/Audio.h"
#include "Engine/Supervisor.h"

#define MENU_MAX_VISIBLE_ENTRIES 11
#define MENU_LINE_HEIGHT 24
#define MENU_FIRST_LINE_Y 120

class DebugMenu
{
public:
	enum class Page {
		ROOT,
		RUNTIME,
		ANIMATION,
		PROFILER,
		CONFIGURATION,
		CONFIGURATION_VIDEO,
		CONFIGURATION_AUDIO,
		CONFIGURATION_GAMEPLAY,
		SCORE,
		SCORE_PSCD,
		SCORE_PSCD_DIFFICULTY,
		SCORE_PSCD_STAGE,
		SCORE_CLRD,
		SCORE_CLRD_MODE,
		SCORE_CLRD_DIFFICULTY,
		SCORE_HSCD,
		SCORE_HSCD_DIFFICULTY,
		SCORE_HSCD_RANK,
		SCORE_SPCD,
		SCORE_PSTD,
	};

	static void init();
	void static update();
	void static render();
	static void setOpen(bool open);

private:
	struct MenuItem {
		std::function<std::string()> label;
		std::function<void()> onActivate;
		std::function<void(int)> onAdjust;
	};

	struct MenuPageDefinition {
		const char* title = "Debug Menu";
		std::vector<MenuItem> items{};
	};

	static Page currentPage;
	static std::vector<Page> pageStack;
	static std::map<Page, MenuPageDefinition> menuPages;
	static int selectedIndex;
	static int debugTargetFPS;
	static int menuScrollOffset;

	static bool isOpen;
	static bool isInteractable;

	static const char* onOffLabel(bool value);
	static bool isFineAdjustHeld();
	static void syncMenuScrollWithSelection(int entryCount, int selected);
	static bool hasConfigFlag(GameConfigFlags flag);
	static void setConfigFlag(GameConfigFlags flag, bool enabled);
	static void requestFullscreenState(bool shouldBeFullscreen);
	static void requestWindowRecreate();
	static const char* getMusicModeLabel(uint8_t mode);
	static void cycleMusicMode(int direction);
	static const char* getDifficultyLabel(uint8_t difficulty);
	static void cycleDefaultDifficulty(int direction);
	static MenuItem createBackMenuItem();
	static const char* getStageLabel(uint8_t stage);
	static void setSfxEnabled(bool enabled);

	static void rebuildMenuModel();
	static MenuPageDefinition* getPageDefinition(Page page);
	static int getEntryCount(Page page);
	static void resetSelection();
	static void goToSubmenu(Page page);
	static void goBack();
	static void adjustCurrentOption(int direction);
	static void activateCurrentOption();
	static std::string getLine(Page page, int index);
	static const char* getPageTitle(Page page);
};

