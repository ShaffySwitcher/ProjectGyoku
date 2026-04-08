#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

class DebugMenu
{
public:
	enum class Page {
		ROOT,
		RUNTIME,
		GRAPHICS,
		ANIMATION,
		PROFILER
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

	static bool isOpen;
	static bool isInteractable;

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

