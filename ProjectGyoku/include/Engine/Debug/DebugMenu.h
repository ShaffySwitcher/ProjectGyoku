#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "Engine/Supervisor.h"

class DebugMenu
{
public:
    static void init();
    static void update();
    static void render();
    static void setOpen(bool open);

    static bool isOpen;

private:
    struct Entry
    {
        enum class Kind { Nav, Value, Action };

        Kind kind;
        std::function<std::string()> label;
        std::function<void()> navigate;
        std::function<void(int)> adjust;
        std::function<void()> activate;

        static Entry nav(std::function<std::string()> label, std::function<void()> navigate);
        static Entry value(std::function<std::string()> label, std::function<void(int)> adjust);
        static Entry action(std::function<std::string()> label, std::function<void()> activate);
        static Entry back();
    };

    struct Page
    {
        std::string title;
        std::vector<Entry> entries;
    };

    using PageFactory = std::function<Page()>;

    static std::vector<Page> pageStack;
    static std::vector<PageFactory> factoryStack;

    static int selectedIndex;
    static int scrollOffset;
    static int debugTargetFPS;
    static bool isInteractable;

    static void push(PageFactory factory);
    static void pop();
    static Page& current();

    static void resetSelection();
    static void syncScroll();

    static bool isFineAdjustHeld();

    static const char* onOffLabel(bool value);
    static const char* musicModeLabel(uint8_t mode);
    static const char* difficultyLabel(uint8_t difficulty);
    static const char* stageLabel(uint8_t stage);
    static std::string characterLabel(uint8_t character);

    static bool hasFlag(GameConfigFlags flag);
    static void setFlag(GameConfigFlags flag, bool enabled);
    static void requestFullscreen(bool enabled);
    static void requestWindowRecreate();
    static void cycleMusicMode(int direction);
    static void cycleDefaultDifficulty(int direction);
    static void setSfxEnabled(bool enabled);

    static PageFactory makeRoot();
    static PageFactory makeRuntime();
    static PageFactory makeAnimation();
    static PageFactory makeProfiler();
    static PageFactory makeConfiguration();
    static PageFactory makeConfigVideo();
    static PageFactory makeConfigAudio();
    static PageFactory makeConfigGameplay();

    static PageFactory makeScore();
    static PageFactory makeScorePSCD();
    static PageFactory makeScorePSCDDifficulty(int character);
    static PageFactory makeScorePSCDStage(int character, int difficulty);
    static PageFactory makeScoreCLRD();
    static PageFactory makeScoreCLRDMode(int character);
    static PageFactory makeScoreCLRDDifficulty(int character, bool withContinues);
    static PageFactory makeScoreHSCD();
    static PageFactory makeScoreHSCDDifficulty(int character);
    static PageFactory makeScoreHSCDRank(int character, int difficulty);
    static PageFactory makeScoreSPCD();
    static PageFactory makeScorePSTD();

    static PageFactory makeGame();
    static PageFactory makeGameConfiguration();
    static PageFactory makeGameStats();

    static constexpr int MAX_VISIBLE = 11;
    static constexpr int LINE_H = 24;
    static constexpr int FIRST_LINE_Y = 120;
};