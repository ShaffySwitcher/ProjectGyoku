#include "Engine/Debug/DebugMenu.h"

#include <DxLib.h>

#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

#include "Engine/Audio/Audio.h"
#include "Engine/Debug/Profiler.h"
#include "Engine/Global.h"
#include "Engine/Graphics/Animation.h"
#include "Engine/Input.h"
#include "Engine/Math/FPS.h"
#include "Engine/Score.h"
#include "Engine/State.h"
#include "Engine/Supervisor.h"
#include "Engine/Utils.h"

#include "Scene/Game.h"

bool DebugMenu::isOpen = false;
std::vector<DebugMenu::Page> DebugMenu::pageStack{};
std::vector<DebugMenu::PageFactory> DebugMenu::factoryStack{};
int DebugMenu::selectedIndex = 0;
int DebugMenu::scrollOffset = 0;
int DebugMenu::debugTargetFPS = 60;
bool DebugMenu::isInteractable = false;

DebugMenu::Entry DebugMenu::Entry::nav(std::function<std::string()> label, std::function<void()> navigate)
{
    Entry entry{};
    entry.kind = Kind::Nav;
    entry.label = std::move(label);
    entry.navigate = std::move(navigate);
    return entry;
}

DebugMenu::Entry DebugMenu::Entry::value(std::function<std::string()> label, std::function<void(int)> adjust)
{
    Entry entry{};
    entry.kind = Kind::Value;
    entry.label = std::move(label);
    entry.adjust = std::move(adjust);
    return entry;
}

DebugMenu::Entry DebugMenu::Entry::action(std::function<std::string()> label, std::function<void()> activate)
{
    Entry entry{};
    entry.kind = Kind::Action;
    entry.label = std::move(label);
    entry.activate = std::move(activate);
    return entry;
}

DebugMenu::Entry DebugMenu::Entry::back()
{
    return nav([]() { return std::string("< Back"); }, []() { DebugMenu::pop(); });
}

const char* DebugMenu::onOffLabel(bool value)
{
    return value ? "ON" : "OFF";
}

bool DebugMenu::isFineAdjustHeld()
{
    return Input::inputCurrent[KEY_INPUT_LSHIFT] || Input::inputCurrent[KEY_INPUT_RSHIFT];
}

bool DebugMenu::hasFlag(GameConfigFlags flag)
{
    const uint16_t mask = static_cast<uint16_t>(flag);
    return (gSupervisor.config.flags & mask) != 0;
}

void DebugMenu::setFlag(GameConfigFlags flag, bool enabled)
{
    const uint16_t mask = static_cast<uint16_t>(flag);

    if (enabled) {
        gSupervisor.config.flags = static_cast<uint16_t>(gSupervisor.config.flags | mask);
    }
    else {
        gSupervisor.config.flags = static_cast<uint16_t>(gSupervisor.config.flags & static_cast<uint16_t>(~mask));
    }
}

void DebugMenu::requestFullscreen(bool enabled)
{
    const bool fullscreenEnabled = hasFlag(GameConfigFlags::FULLSCREEN);
    if (fullscreenEnabled != enabled) {
        gSupervisor.wantFullscreen = true;
    }
}

void DebugMenu::requestWindowRecreate()
{
    gSupervisor.wantWindowRecreate = true;
}

const char* DebugMenu::musicModeLabel(uint8_t mode)
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

const char* DebugMenu::difficultyLabel(uint8_t difficulty)
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

const char* DebugMenu::stageLabel(uint8_t stage)
{
    switch (static_cast<Stage>(stage)) {
    case Stage::STAGE_1:
        return "STAGE 1";
    case Stage::STAGE_2:
        return "STAGE 2";
    case Stage::STAGE_3:
        return "STAGE 3";
    case Stage::STAGE_4:
        return "STAGE 4";
    case Stage::STAGE_5:
        return "STAGE 5";
    case Stage::STAGE_6:
        return "STAGE 6";
    case Stage::EXTRA:
        return "EXTRA";
    default:
        return "UNKNOWN";
    }
}

std::string DebugMenu::characterLabel(uint8_t character)
{
    std::string charName;
    std::string shotName;

    uint8_t charIndex = character % static_cast<uint8_t>(Character::COUNT);
    uint8_t shotIndex = character / static_cast<uint8_t>(Character::COUNT);

    switch (static_cast<Character>(charIndex)) {
    case Character::LLOYD:
        charName = "LLOYD";
        break;
    default:
        charName = "UNKNOWN";
        break;
    }

    switch (static_cast<ShotType>(shotIndex)) {
    case ShotType::SHOT_TYPE_A:
        shotName = "(A)";
        break;
    case ShotType::SHOT_TYPE_B:
        shotName = "(B)";
        break;
    case ShotType::SHOT_TYPE_C:
        shotName = "(C)";
        break;
    default:
        shotName = "(?)";
        break;
    }

    return charName + " " + shotName;
}

void DebugMenu::setSfxEnabled(bool enabled)
{
    gSupervisor.config.useSfx = enabled;

    if (!gSupervisor.config.useSfx) {
        SFXPlayer::stopAll();
    }
}

void DebugMenu::push(PageFactory factory)
{
    if (!factory) {
        return;
    }

    factoryStack.push_back(factory);
    pageStack.push_back(factory());
    resetSelection();
}

void DebugMenu::pop()
{
    if (pageStack.size() > 1 && factoryStack.size() > 1) {
        pageStack.pop_back();
        factoryStack.pop_back();
    }

    resetSelection();
}

DebugMenu::Page& DebugMenu::current()
{
    static Page emptyPage;
    if (emptyPage.title.empty()) {
        emptyPage.title = "Debug Menu";
    }

    if (pageStack.empty()) {
        return emptyPage;
    }

    return pageStack.back();
}

void DebugMenu::resetSelection()
{
    selectedIndex = 0;
    scrollOffset = 0;
}

void DebugMenu::syncScroll()
{
    const int count = static_cast<int>(current().entries.size());
    if (count <= 0) {
        scrollOffset = 0;
        return;
    }

    const int maxOffset = (count > MAX_VISIBLE) ? (count - MAX_VISIBLE) : 0;
    scrollOffset = clamp(scrollOffset, 0, maxOffset);

    if (selectedIndex < scrollOffset) {
        scrollOffset = selectedIndex;
    }
    else if (selectedIndex >= scrollOffset + MAX_VISIBLE) {
        scrollOffset = selectedIndex - MAX_VISIBLE + 1;
    }

    scrollOffset = clamp(scrollOffset, 0, maxOffset);
}

DebugMenu::PageFactory DebugMenu::makeRoot()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu";

        page.entries.push_back(Entry::nav([]() { return std::string("Game >"); }, []() { DebugMenu::push(DebugMenu::makeGame()); }));
        page.entries.push_back(Entry::nav([]() { return std::string("Runtime >"); }, []() { DebugMenu::push(DebugMenu::makeRuntime()); }));
        page.entries.push_back(Entry::nav([]() { return std::string("Animation / ANM >"); }, []() { DebugMenu::push(DebugMenu::makeAnimation()); }));
        page.entries.push_back(Entry::nav([]() { return std::string("Profiler >"); }, []() { DebugMenu::push(DebugMenu::makeProfiler()); }));
        page.entries.push_back(Entry::nav([]() { return std::string("Configuration >"); }, []() { DebugMenu::push(DebugMenu::makeConfiguration()); }));
        page.entries.push_back(Entry::nav([]() { return std::string("Score >"); }, []() { DebugMenu::push(DebugMenu::makeScore()); }));
        page.entries.push_back(Entry::action([]() { return std::string("Close menu"); }, []() { DebugMenu::setOpen(false); }));

        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeRuntime()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Runtime";

        page.entries.push_back(Entry::value(
            []() { return std::string("Target FPS: ") + std::to_string(DebugMenu::debugTargetFPS); },
            [](int direction) {
                const int step = DebugMenu::isFineAdjustHeld() ? 1 : 5;
                DebugMenu::debugTargetFPS = clamp(DebugMenu::debugTargetFPS + (direction * step), 30, 240);
                FPS::setFPS(DebugMenu::debugTargetFPS);
            }
        ));

        page.entries.push_back(Entry::value(
            []() { return format("Game Speed: %.2f", gGameManager.gameSpeed); },
            [](int direction) {
                const float step = DebugMenu::isFineAdjustHeld() ? 0.01f : 0.1f;
                gGameManager.gameSpeed = clamp(gGameManager.gameSpeed + (direction * step), 0.1f, 4.0f);
            }
        ));

        page.entries.push_back(Entry::action([]() { return std::string("Reset Game Speed"); }, []() { gGameManager.gameSpeed = 1.0f; }));
        page.entries.push_back(Entry::action([]() { return std::string("Reset Global Frame Counter"); }, []() { gSupervisor.currentFrame = 0; }));
        page.entries.push_back(Entry::back());

        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeAnimation()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Animation";

        page.entries.push_back(Entry::action([]() { return std::string("Reload scripts"); }, []() { ANMManager::reloadScriptsAndRestartRunners(); }));
        page.entries.push_back(Entry::action([]() { return std::string("Reload scripts (in-place)"); }, []() { ANMManager::reloadScripts(); }));
        page.entries.push_back(Entry::action([]() { return std::string("Reload textures"); }, []() { ANMManager::reloadTextures(); }));
        page.entries.push_back(Entry::action([]() { return std::string("Unload all"); }, []() { ANMManager::unloadAll(); }));
        page.entries.push_back(Entry::back());

        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeProfiler()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Profiler";

        page.entries.push_back(Entry::value(
            []() { return std::string("Enabled: ") + DebugMenu::onOffLabel(Profiler::isEnabled()); },
            [](int direction) { Profiler::setEnabled(direction > 0); }
        ));

        page.entries.push_back(Entry::value(
            []() { return std::string("Overlay: ") + DebugMenu::onOffLabel(Profiler::isOverlayEnabled()); },
            [](int direction) { Profiler::setOverlayEnabled(direction > 0); }
        ));

        page.entries.push_back(Entry::action([]() { return format("Frame: %.2f ms", Profiler::getLastFrameMilliseconds()); }, []() {}));
        page.entries.push_back(Entry::action([]() { return format("Loop: %.2f ms", Profiler::getLastSampleMilliseconds("Loop")); }, []() {}));
        page.entries.push_back(Entry::action([]() { return format("State Update: %.2f ms", Profiler::getLastSampleMilliseconds("State Update")); }, []() {}));
        page.entries.push_back(Entry::action([]() { return format("State Render: %.2f ms", Profiler::getLastSampleMilliseconds("State Render")); }, []() {}));
        page.entries.push_back(Entry::back());

        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeConfiguration()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Configuration";

        page.entries.push_back(Entry::nav([]() { return std::string("Video >"); }, []() { DebugMenu::push(DebugMenu::makeConfigVideo()); }));
        page.entries.push_back(Entry::nav([]() { return std::string("Audio >"); }, []() { DebugMenu::push(DebugMenu::makeConfigAudio()); }));
        page.entries.push_back(Entry::nav([]() { return std::string("Gameplay >"); }, []() { DebugMenu::push(DebugMenu::makeConfigGameplay()); }));
        page.entries.push_back(Entry::action([]() { return std::string("Reload configuration from file"); }, []() { gSupervisor.loadConfig("pg01.cfg"); }));
        page.entries.push_back(Entry::action([]() { return std::string("Save current configuration to file"); }, []() { gSupervisor.saveConfig("pg01.cfg"); }));
        page.entries.push_back(Entry::action([]() { return std::string("Reset to default configuration"); }, []() { gSupervisor.setDefaultConfig(); }));
        page.entries.push_back(Entry::back());

        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeConfigVideo()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Configuration / Video";

        page.entries.push_back(Entry::value(
            []() { return std::string("Fullscreen: ") + DebugMenu::onOffLabel(DebugMenu::hasFlag(GameConfigFlags::FULLSCREEN)); },
            [](int direction) { DebugMenu::requestFullscreen(direction > 0); }
        ));

        page.entries.push_back(Entry::value(
            []() { return std::string("VSync: ") + DebugMenu::onOffLabel(DebugMenu::hasFlag(GameConfigFlags::VSYNC)); },
            [](int direction) { DebugMenu::setFlag(GameConfigFlags::VSYNC, direction > 0); }
        ));

        page.entries.push_back(Entry::value(
            []() { return std::string("Force Refresh Rate: ") + DebugMenu::onOffLabel(DebugMenu::hasFlag(GameConfigFlags::FORCE_REFRESH_RATE)); },
            [](int direction) { DebugMenu::setFlag(GameConfigFlags::FORCE_REFRESH_RATE, direction > 0); }
        ));

        page.entries.push_back(Entry::value(
            []() { return std::string("16-bit Textures: ") + DebugMenu::onOffLabel(DebugMenu::hasFlag(GameConfigFlags::USE_16_BIT_TEXTURES)); },
            [](int direction) { DebugMenu::setFlag(GameConfigFlags::USE_16_BIT_TEXTURES, direction > 0); }
        ));

        page.entries.push_back(Entry::value(
            []() { return std::string("Disable Fog: ") + DebugMenu::onOffLabel(DebugMenu::hasFlag(GameConfigFlags::DISABLE_FOG)); },
            [](int direction) { DebugMenu::setFlag(GameConfigFlags::DISABLE_FOG, direction > 0); }
        ));

        page.entries.push_back(Entry::value(
            []() { return format("Window Scale: %.2f", gSupervisor.config.windowScale); },
            [](int direction) {
                const float step = DebugMenu::isFineAdjustHeld() ? 0.01f : 0.1f;
                gSupervisor.config.windowScale = clamp(gSupervisor.config.windowScale + (direction * step), 0.5f, 4.0f);
            }
        ));

        page.entries.push_back(Entry::action([]() { return std::string("Recreate Window Now"); }, []() { DebugMenu::requestWindowRecreate(); }));
        page.entries.push_back(Entry::back());

        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeConfigAudio()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Configuration / Audio";

        page.entries.push_back(Entry::value(
            []() { return std::string("BGM Mode: ") + DebugMenu::musicModeLabel(gSupervisor.config.bgmType); },
            [](int direction) { DebugMenu::cycleMusicMode(direction); }
        ));

        page.entries.push_back(Entry::value(
            []() { return std::string("BGM Volume: ") + std::to_string(static_cast<int>(gSupervisor.config.bgmVolume)); },
            [](int direction) {
                int volume = static_cast<int>(gSupervisor.config.bgmVolume);
                const int step = DebugMenu::isFineAdjustHeld() ? 1 : 5;
                volume = clamp(volume + (direction * step), 0, 100);
                gSupervisor.config.bgmVolume = static_cast<uint8_t>(volume);
            }
        ));

        page.entries.push_back(Entry::value(
            []() { return std::string("SFX Enabled: ") + DebugMenu::onOffLabel(gSupervisor.config.useSfx); },
            [](int direction) { DebugMenu::setSfxEnabled(direction > 0); }
        ));

        page.entries.push_back(Entry::value(
            []() { return std::string("SFX Volume: ") + std::to_string(static_cast<int>(gSupervisor.config.sfxVolume)); },
            [](int direction) {
                int volume = static_cast<int>(gSupervisor.config.sfxVolume);
                const int step = DebugMenu::isFineAdjustHeld() ? 1 : 5;
                volume = clamp(volume + (direction * step), 0, 100);
                gSupervisor.config.sfxVolume = static_cast<uint8_t>(volume);
            }
        ));

        page.entries.push_back(Entry::back());

        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeConfigGameplay()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Configuration / Gameplay";

        page.entries.push_back(Entry::value(
            []() { return std::string("Default Difficulty: ") + DebugMenu::difficultyLabel(gSupervisor.config.defaultDifficulty); },
            [](int direction) { DebugMenu::cycleDefaultDifficulty(direction); }
        ));

        page.entries.push_back(Entry::value(
            []() { return std::string("Frame Skip: ") + std::to_string(static_cast<int>(gSupervisor.config.frameSkip)); },
            [](int direction) {
                int frameSkip = static_cast<int>(gSupervisor.config.frameSkip);
                frameSkip = clamp(frameSkip + direction, 0, 8);
                gSupervisor.config.frameSkip = static_cast<uint8_t>(frameSkip);
            }
        ));

        page.entries.push_back(Entry::back());

        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeScore()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Score";

        page.entries.push_back(Entry::nav([]() { return std::string("PSCD (Player Stage Clear Data) >"); }, []() { DebugMenu::push(DebugMenu::makeScorePSCD()); }));
        page.entries.push_back(Entry::nav([]() { return std::string("CLRD (Clear Data) >"); }, []() { DebugMenu::push(DebugMenu::makeScoreCLRD()); }));
        page.entries.push_back(Entry::nav([]() { return std::string("HSCD (Highscore Data) >"); }, []() { DebugMenu::push(DebugMenu::makeScoreHSCD()); }));
        page.entries.push_back(Entry::nav([]() { return std::string("SPCD (Spellcard Data) >"); }, []() { DebugMenu::push(DebugMenu::makeScoreSPCD()); }));
        page.entries.push_back(Entry::nav([]() { return std::string("PSTD (Gameplay Data) >"); }, []() { DebugMenu::push(DebugMenu::makeScorePSTD()); }));
        page.entries.push_back(Entry::action([]() { return std::string("Save Score Data Now"); }, []() {
            if (ScoreManager::getCurrentScore()) {
                ScoreManager::save("score.dat");
            }
        }));
        page.entries.push_back(Entry::action([]() { return std::string("Reset Score Data"); }, []() {
            if (ScoreManager::getCurrentScore()) {
                ScoreManager::getCurrentScore()->reset();
                ScoreManager::save("score.dat");
            }
        }));
        page.entries.push_back(Entry::back());

        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeScorePSCD()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Score / PSCD";

        if (!ScoreManager::getCurrentScore()) {
            page.entries.push_back(Entry::action([]() { return std::string("(no score data)"); }, []() {}));
        }
        else {
            for (int character = 0; character < CHARACTER_COUNT; ++character) {
                const int characterCopy = character;
                page.entries.push_back(Entry::nav(
                    [characterCopy]() { return format("Character: %s >", DebugMenu::characterLabel(static_cast<uint8_t>(characterCopy)).c_str()); },
                    [characterCopy]() { DebugMenu::push(DebugMenu::makeScorePSCDDifficulty(characterCopy)); }
                ));
            }
        }

        page.entries.push_back(Entry::back());
        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeScorePSCDDifficulty(int character)
{
    return [character]() -> Page {
        Page page;
        page.title = format("Debug Menu / Score / PSCD / Character %d", character + 1);

        if (!ScoreManager::getCurrentScore()) {
            page.entries.push_back(Entry::action([]() { return std::string("(no score data)"); }, []() {}));
        }
        else {
            for (int difficulty = 0; difficulty < DIFFICULTY_COUNT; ++difficulty) {
                const int difficultyCopy = difficulty;
                page.entries.push_back(Entry::nav(
                    [difficultyCopy]() { return format("%s >", DebugMenu::difficultyLabel(static_cast<uint8_t>(difficultyCopy))); },
                    [character, difficultyCopy]() { DebugMenu::push(DebugMenu::makeScorePSCDStage(character, difficultyCopy)); }
                ));
            }
        }

        page.entries.push_back(Entry::back());
        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeScorePSCDStage(int character, int difficulty)
{
    return [character, difficulty]() -> Page {
        Page page;
        page.title = format("Debug Menu / Score / PSCD / Character %d / %s", character + 1, DebugMenu::difficultyLabel(static_cast<uint8_t>(difficulty)));

        const Score* score = ScoreManager::getCurrentScore();
        if (!score) {
            page.entries.push_back(Entry::action([]() { return std::string("(no score data)"); }, []() {}));
        }
        else {
            for (int stage = 0; stage < STAGE_COUNT; ++stage) {
                const int stageCopy = stage;
                page.entries.push_back(Entry::value(
                    [character, difficulty, stageCopy]() -> std::string {
                        const Score* currentScore = ScoreManager::getCurrentScore();
                        if (!currentScore) {
                            return "(null)";
                        }

                        const PSCD& entry = currentScore->playerStageClearData[character][difficulty][stageCopy];
                        return format("%-8s  %u", DebugMenu::stageLabel(static_cast<uint8_t>(stageCopy)), entry.score);
                    },
                    [character, difficulty, stageCopy](int direction) {
                        Score* currentScore = ScoreManager::getCurrentScore();
                        if (!currentScore) {
                            return;
                        }

                        PSCD& entry = currentScore->playerStageClearData[character][difficulty][stageCopy];
                        const int64_t step = DebugMenu::isFineAdjustHeld() ? 1000 : 10000;
                        const int64_t next = static_cast<int64_t>(entry.score) + (static_cast<int64_t>(direction) * step);
                        entry.score = static_cast<uint32_t>(clamp(next, static_cast<int64_t>(0), static_cast<int64_t>(0xFFFFFFFFu)));
                    }
                ));
            }
        }

        page.entries.push_back(Entry::back());
        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeScoreCLRD()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Score / CLRD";

        if (!ScoreManager::getCurrentScore()) {
            page.entries.push_back(Entry::action([]() { return std::string("(no score data)"); }, []() {}));
        }
        else {
            for (int character = 0; character < CHARACTER_COUNT; ++character) {
                const int characterCopy = character;
                page.entries.push_back(Entry::nav(
                    [characterCopy]() { return format("Character: %s >", DebugMenu::characterLabel(static_cast<uint8_t>(characterCopy)).c_str()); },
                    [characterCopy]() { DebugMenu::push(DebugMenu::makeScoreCLRDMode(characterCopy)); }
                ));
            }
        }

        page.entries.push_back(Entry::back());
        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeScoreCLRDMode(int character)
{
    return [character]() -> Page {
        Page page;
        page.title = format("Debug Menu / Score / CLRD / Character %d", character + 1);

        if (!ScoreManager::getCurrentScore()) {
            page.entries.push_back(Entry::action([]() { return std::string("(no score data)"); }, []() {}));
        }
        else {
            page.entries.push_back(Entry::nav([]() { return std::string("No Continues >"); }, [character]() { DebugMenu::push(DebugMenu::makeScoreCLRDDifficulty(character, false)); }));
            page.entries.push_back(Entry::nav([]() { return std::string("With Continues >"); }, [character]() { DebugMenu::push(DebugMenu::makeScoreCLRDDifficulty(character, true)); }));
        }

        page.entries.push_back(Entry::back());
        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeScoreCLRDDifficulty(int character, bool withContinues)
{
    return [character, withContinues]() -> Page {
        Page page;
        page.title = format(
            "Debug Menu / Score / CLRD / Character %d / %s",
            character + 1,
            withContinues ? "With Continues" : "No Continues"
        );

        const Score* score = ScoreManager::getCurrentScore();
        if (!score) {
            page.entries.push_back(Entry::action([]() { return std::string("(no score data)"); }, []() {}));
        }
        else {
            for (int difficulty = 0; difficulty < DIFFICULTY_COUNT; ++difficulty) {
                const int difficultyCopy = difficulty;
                page.entries.push_back(Entry::value(
                    [character, difficultyCopy, withContinues]() -> std::string {
                        const Score* currentScore = ScoreManager::getCurrentScore();
                        if (!currentScore) {
                            return "(null)";
                        }

                        const CLRD& entry = currentScore->clearData[character];
                        const uint8_t value = withContinues ? entry.stageClearedWithContinues[difficultyCopy] : entry.stageCleared[difficultyCopy];
                        return format("%-8s  %s", DebugMenu::difficultyLabel(static_cast<uint8_t>(difficultyCopy)), DebugMenu::stageLabel(value));
                    },
                    [character, difficultyCopy, withContinues](int direction) {
                        Score* currentScore = ScoreManager::getCurrentScore();
                        if (!currentScore) {
                            return;
                        }

                        CLRD& entry = currentScore->clearData[character];
                        uint8_t& value = withContinues ? entry.stageClearedWithContinues[difficultyCopy] : entry.stageCleared[difficultyCopy];
                        const int next = static_cast<int>(value) + direction;
                        value = static_cast<uint8_t>(clamp(next, 0, static_cast<int>(STAGE_COUNT) - 1));
                    }
                ));
            }
        }

        page.entries.push_back(Entry::back());
        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeScoreHSCD()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Score / HSCD";

        if (!ScoreManager::getCurrentScore()) {
            page.entries.push_back(Entry::action([]() { return std::string("(no score data)"); }, []() {}));
        }
        else {
            for (int character = 0; character < CHARACTER_COUNT; ++character) {
                const int characterCopy = character;
                page.entries.push_back(Entry::nav(
                    [characterCopy]() { return format("Character: %s >", DebugMenu::characterLabel(static_cast<uint8_t>(characterCopy)).c_str()); },
                    [characterCopy]() { DebugMenu::push(DebugMenu::makeScoreHSCDDifficulty(characterCopy)); }
                ));
            }
        }

        page.entries.push_back(Entry::back());
        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeScoreHSCDDifficulty(int character)
{
    return [character]() -> Page {
        Page page;
        page.title = format("Debug Menu / Score / HSCD / Character %d", character + 1);

        if (!ScoreManager::getCurrentScore()) {
            page.entries.push_back(Entry::action([]() { return std::string("(no score data)"); }, []() {}));
        }
        else {
            for (int difficulty = 0; difficulty < DIFFICULTY_COUNT; ++difficulty) {
                const int difficultyCopy = difficulty;
                page.entries.push_back(Entry::nav(
                    [difficultyCopy]() { return format("%s >", DebugMenu::difficultyLabel(static_cast<uint8_t>(difficultyCopy))); },
                    [character, difficultyCopy]() { DebugMenu::push(DebugMenu::makeScoreHSCDRank(character, difficultyCopy)); }
                ));
            }
        }

        page.entries.push_back(Entry::back());
        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeScoreHSCDRank(int character, int difficulty)
{
    return [character, difficulty]() -> Page {
        Page page;
        page.title = format("Debug Menu / Score / HSCD / Character %d / %s", character + 1, DebugMenu::difficultyLabel(static_cast<uint8_t>(difficulty)));

        const Score* score = ScoreManager::getCurrentScore();
        if (!score) {
            page.entries.push_back(Entry::action([]() { return std::string("(no score data)"); }, []() {}));
        }
        else {
            for (int rank = 0; rank < HIGHSCORE_COUNT; ++rank) {
                const int rankCopy = rank;
                page.entries.push_back(Entry::value(
                    [character, difficulty, rankCopy]() -> std::string {
                        const Score* currentScore = ScoreManager::getCurrentScore();
                        if (!currentScore) {
                            return "(null)";
                        }

                        const HSCD& entry = currentScore->highscores[character][difficulty][rankCopy];
                        char name[9] = {};
                        std::memcpy(name, entry.name, 8);

                        const char* stateLabel = "UNKNOWN";
                        switch (static_cast<HighscoreState>(entry.state)) {
                        case HighscoreState::UNINITIALIZED:
                            stateLabel = "UNINIT";
                            break;
                        case HighscoreState::DEFAULT:
                            stateLabel = "DEFAULT";
                            break;
                        case HighscoreState::USER:
                            stateLabel = "USER";
                            break;
                        }

                        return format("#%-2d  %-8s  %u  [%s]", rankCopy + 1, name, entry.score, stateLabel);
                    },
                    [character, difficulty, rankCopy](int direction) {
                        Score* currentScore = ScoreManager::getCurrentScore();
                        if (!currentScore) {
                            return;
                        }

                        HSCD& entry = currentScore->highscores[character][difficulty][rankCopy];
                        const int64_t step = DebugMenu::isFineAdjustHeld() ? 1000 : 10000;
                        const int64_t next = static_cast<int64_t>(entry.score) + (static_cast<int64_t>(direction) * step);
                        entry.score = static_cast<uint32_t>(clamp(next, static_cast<int64_t>(0), static_cast<int64_t>(0xFFFFFFFFu)));
                        entry.state = static_cast<uint8_t>(HighscoreState::USER);
                    }
                ));
            }
        }

        page.entries.push_back(Entry::back());
        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeScoreSPCD()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Score / SPCD";

        const Score* score = ScoreManager::getCurrentScore();
        if (!score) {
            page.entries.push_back(Entry::action([]() { return std::string("(no score data)"); }, []() {}));
        }
        else {
            for (int spellcard = 0; spellcard < SPELLCARD_COUNT; ++spellcard) {
                const int spellcardCopy = spellcard;
                page.entries.push_back(Entry::value(
                    [spellcardCopy]() -> std::string {
                        const Score* currentScore = ScoreManager::getCurrentScore();
                        if (!currentScore) {
                            return "(null)";
                        }

                        const SPCD& sp = currentScore->spellcards[spellcardCopy];
                        char name[65] = {};
                        std::memcpy(name, sp.name, 64);

                        const float rate = (sp.attempts > 0)
                            ? (static_cast<float>(sp.success) / static_cast<float>(sp.attempts) * 100.0f)
                            : 0.0f;

                        return format(
                            "#%-3d  %-20s  %u/%u (%.0f%%)  score:%u",
                            spellcardCopy + 1,
                            name,
                            static_cast<unsigned>(sp.success),
                            static_cast<unsigned>(sp.attempts),
                            rate,
                            sp.score
                        );
                    },
                    [spellcardCopy](int direction) {
                        Score* currentScore = ScoreManager::getCurrentScore();
                        if (!currentScore) {
                            return;
                        }

                        SPCD& sp = currentScore->spellcards[spellcardCopy];
                        const int64_t step = DebugMenu::isFineAdjustHeld() ? 1000 : 10000;
                        const int64_t next = static_cast<int64_t>(sp.score) + (static_cast<int64_t>(direction) * step);
                        sp.score = static_cast<uint32_t>(clamp(next, static_cast<int64_t>(0), static_cast<int64_t>(0xFFFFFFFFu)));
                    }
                ));
            }
        }

        page.entries.push_back(Entry::back());
        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeScorePSTD()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Score / PSTD";

        static const char* const diffLabels[] = { "Easy", "Normal", "Hard", "Illusory", "Extra" };

        const Score* score = ScoreManager::getCurrentScore();
        if (!score) {
            page.entries.push_back(Entry::action([]() { return std::string("(no score data)"); }, []() {}));
        }
        else {
            page.entries.push_back(Entry::action(
                []() -> std::string {
                    const Score* currentScore = ScoreManager::getCurrentScore();
                    if (!currentScore) {
                        return "(null)";
                    }

                    const PSTD& ps = currentScore->playStats;
                    const uint32_t th = ps.timePlayed.hours;
                    const uint32_t tm = ps.timePlayed.minutes;
                    const uint32_t ts = ps.timePlayed.seconds;
                    const uint32_t gh = ps.timePlayedGame.hours;
                    const uint32_t gm = ps.timePlayedGame.minutes;
                    const uint32_t gs = ps.timePlayedGame.seconds;
                    return format("Total time:  %02u:%02u:%02u  (in-game: %02u:%02u:%02u)", th, tm, ts, gh, gm, gs);
                },
                []() {}
            ));

            for (int difficulty = 0; difficulty < DIFFICULTY_COUNT; ++difficulty) {
                const int difficultyCopy = difficulty;
                page.entries.push_back(Entry::action(
                    [difficultyCopy]() -> std::string {
                        const Score* currentScore = ScoreManager::getCurrentScore();
                        if (!currentScore) {
                            return "(null)";
                        }

                        const GPLD& g = currentScore->playStats.dataDifficulty[difficultyCopy];
                        uint32_t totalPlays = 0;
                        for (int character = 0; character < CHARACTER_COUNT; ++character) {
                            totalPlays += g.plays[character];
                        }

                        return format("%-8s  plays:%u  clears:%u  cont:%u  prac:%u", diffLabels[difficultyCopy], totalPlays, g.clears, g.continues, g.practices);
                    },
                    []() {}
                ));
            }

            page.entries.push_back(Entry::action(
                []() -> std::string {
                    const Score* currentScore = ScoreManager::getCurrentScore();
                    if (!currentScore) {
                        return "(null)";
                    }

                    const GPLD& g = currentScore->playStats.dataTotal;
                    uint32_t totalPlays = 0;
                    for (int character = 0; character < CHARACTER_COUNT; ++character) {
                        totalPlays += g.plays[character];
                    }

                    return format("TOTAL     plays:%u  clears:%u  cont:%u  prac:%u", totalPlays, g.clears, g.continues, g.practices);
                },
                []() {}
            ));
        }

        page.entries.push_back(Entry::back());
        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeGame()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Game";

        page.entries.push_back(Entry::nav([]() { return std::string("Configuration >"); }, []() { DebugMenu::push(DebugMenu::makeGameConfiguration()); }));
        page.entries.push_back(Entry::nav([]() { return std::string("Stats >"); }, []() { DebugMenu::push(DebugMenu::makeGameStats()); }));
        page.entries.push_back(Entry::action([]() { return std::string("Kill Player"); }, []() { std::dynamic_pointer_cast<Game>(gStateManager.getState())->getPlayer()->collide(); }));
        page.entries.push_back(Entry::back());

        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeGameConfiguration()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Game / Configuration";

        page.entries.push_back(Entry::value(
            []() { return format("Game Speed: %.2f", gGameManager.gameSpeed); },
            [](int direction) {
                const float step = DebugMenu::isFineAdjustHeld() ? 0.01f : 0.1f;
                gGameManager.gameSpeed = clamp(gGameManager.gameSpeed + (direction * step), 0.1f, 4.0f);
            }
        ));
        page.entries.push_back(Entry::action([]() { return std::string("Reset Game Frame"); }, []() { gGameManager.frame = 0; }));
        page.entries.push_back(Entry::value(
            []() { return format("Character: %s", DebugMenu::characterLabel(gGameManager.character).c_str()); },
            [](int direction) {
                int character = static_cast<int>(gGameManager.character);
                character = (character + direction);
                if (character >= CHARACTER_COUNT) character -= CHARACTER_COUNT;
                if (character < 0) character += CHARACTER_COUNT;
                gGameManager.character = static_cast<uint8_t>(character);
            }
        ));
        page.entries.push_back(Entry::value(
            []() { return format("Difficulty: %s", DebugMenu::difficultyLabel(gGameManager.difficulty)); },
            [](int direction) {
                int difficulty = static_cast<int>(gGameManager.difficulty);
                difficulty = (difficulty + direction);
                if (difficulty >= DIFFICULTY_COUNT) difficulty -= DIFFICULTY_COUNT;
                if (difficulty < 0) difficulty += DIFFICULTY_COUNT;
                gGameManager.difficulty = static_cast<uint8_t>(difficulty);
            }
        ));
        page.entries.push_back(Entry::value(
            []() { return format("Stage: %s", DebugMenu::stageLabel(gGameManager.stage)); },
            [](int direction) {
                int stage = static_cast<int>(gGameManager.stage);
                stage = (stage + direction);
                if (stage >= STAGE_COUNT) stage -= STAGE_COUNT;
                if (stage < 0) stage += STAGE_COUNT;
                gGameManager.stage = static_cast<uint8_t>(stage);
            }
        ));
        page.entries.push_back(Entry::back());

        return page;
    };
}

DebugMenu::PageFactory DebugMenu::makeGameStats()
{
    return []() -> Page {
        Page page;
        page.title = "Debug Menu / Game / Stats";

        page.entries.push_back(Entry::value(
            []() { return format("Player Lives: %d", gGameManager.lives); },
            [](int direction) {
                int lives = gGameManager.lives;
                lives = clamp(lives + direction, 0, 8);
                gGameManager.lives = static_cast<uint8_t>(lives);
            }
        ));

        page.entries.push_back(Entry::value(
            []() { return format("Player Bombs: %d", gGameManager.bombs); },
            [](int direction) {
                int bombs = gGameManager.bombs;
                bombs = clamp(bombs + direction, 0, 8);
                gGameManager.bombs = static_cast<uint8_t>(bombs);
            }
        ));

        page.entries.push_back(Entry::value(
            []() { return format("Player Power: %.1f", gGameManager.power); },
            [](int direction) {
                int power = gGameManager.power;
                power = clamp(power + direction, 0, 128);
                gGameManager.power = power;
            }
        ));

        page.entries.push_back(Entry::value(
            []() { return format("Player Graze: %d", gGameManager.graze); },
            [](int direction) {
                int graze = gGameManager.graze;
                graze = clamp(graze + direction, 0, 65535);
                gGameManager.graze = static_cast<uint16_t>(graze);
            }
        ));

        page.entries.push_back(Entry::value(
            []() { return format("Player Points: %d", gGameManager.points); },
            [](int direction) {
                int points = gGameManager.points;
                points = clamp(points + direction, 0, 65535);
                gGameManager.points = static_cast<uint16_t>(points);
            }
        ));

        page.entries.push_back(Entry::back());

        return page;
    };
}

void DebugMenu::init()
{
    isOpen = false;
    isInteractable = false;
    pageStack.clear();
    factoryStack.clear();
    resetSelection();
    debugTargetFPS = FPS::getFPS();
}

void DebugMenu::setOpen(bool open)
{
    DebugMenu::isOpen = open;
}

void DebugMenu::update()
{
    if (Input::inputPressed[KEY_INPUT_F8]) {
        isInteractable = !isInteractable;
        isOpen = isInteractable;
        if (isInteractable) {
            pageStack.clear();
            factoryStack.clear();
            push(makeRoot());
            debugTargetFPS = FPS::getFPS();
        }
    }

    if (Input::inputPressed[KEY_INPUT_F7])
        isOpen = !isOpen;

    if (!isInteractable || !isOpen) {
        if (isInteractable) Input::clearGameInputState();
        return;
    }

    Page& page = current();
    const int count = static_cast<int>(page.entries.size());
    if (count == 0) { resetSelection(); Input::clearGameInputState(); return; }

    selectedIndex = clamp(selectedIndex, 0, count - 1);

    if (Input::inputPressed[KEY_INPUT_UP]) {
        selectedIndex = (selectedIndex == 0) ? count - 1 : selectedIndex - 1;
    }
    if (Input::inputPressed[KEY_INPUT_DOWN]) {
        selectedIndex = (selectedIndex == count - 1) ? 0 : selectedIndex + 1;
    }

    const Entry& entry = page.entries[selectedIndex];

    if (Input::inputPressed[KEY_INPUT_LEFT] && entry.kind == Entry::Kind::Value)
        entry.adjust(-1);
    if (Input::inputPressed[KEY_INPUT_RIGHT] && entry.kind == Entry::Kind::Value)
        entry.adjust(1);

    if (Input::inputPressed[KEY_INPUT_RETURN] || Input::inputPressed[KEY_INPUT_Z]) {
        if (entry.kind == Entry::Kind::Nav) entry.navigate();
        else if (entry.kind == Entry::Kind::Action) entry.activate();
    }

    if (Input::inputPressed[KEY_INPUT_BACK]
     || Input::inputPressed[KEY_INPUT_X]
     || Input::inputPressed[KEY_INPUT_ESCAPE])
    {
        pop();
    }

    syncScroll();
    Input::clearGameInputState();
}

void DebugMenu::render()
{
    if (!isOpen) {
        return;
    }

    const Page& page = current();
    const int count = static_cast<int>(page.entries.size());
    const int maxOffset = (count > MAX_VISIBLE) ? (count - MAX_VISIBLE) : 0;
    const int firstVisible = clamp(scrollOffset, 0, maxOffset);
    const int lastExclusive = (count < firstVisible + MAX_VISIBLE) ? count : (firstVisible + MAX_VISIBLE);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 204);
    DrawBox(140, 70, 620, 430, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    DrawBox(140, 70, 620, 430, GetColor(190, 190, 190), FALSE);
    DrawString(158, 86, page.title.c_str(), GetColor(255, 255, 255));

    for (int i = firstVisible; i < lastExclusive; ++i) {
        const int y = FIRST_LINE_Y + ((i - firstVisible) * LINE_H);
        const bool selected = isInteractable && (i == selectedIndex);

        if (selected) {
            DrawBox(154, y - 2, 606, y + 18, GetColor(70, 70, 160), TRUE);
        }

        std::string line = page.entries[i].label ? page.entries[i].label() : std::string();
        DrawString(160, y, line.c_str(), GetColor(255, 255, 255));
    }

    if (count > MAX_VISIBLE) {
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

    if (isInteractable) {
        DrawString(158, 398, "Arrows: Move  Enter/Z: Select  Back/X/Esc: Back", GetColor(170, 170, 170));
    }
    else {
        DrawString(158, 398, "F8: enable interactive mode", GetColor(170, 170, 170));
    }
}