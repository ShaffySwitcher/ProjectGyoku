#include "Engine/Debug/DebugMenu.h"
#include <DxLib.h>
#include "Engine/Input.h"
#include "Engine/Graphics/Animation.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Math/FPS.h"
#include "Engine/Global.h"
#include "Engine/Debug/Profiler.h"
#include "Engine/State.h"
#include "Engine/Supervisor.h"
#include "Engine/Utils.h"
#include "Engine/Score.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct ScoreMenuContext {
	int character = 0;
	int difficulty = 0;
	int mode = 0;
};

static ScoreMenuContext gScoreMenuContext{};

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

const char* DebugMenu::getStageLabel(uint8_t stage)
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

DebugMenu::MenuItem DebugMenu::createBackMenuItem()
{
	return { []() { return std::string("< Back"); }, []() { goBack(); }, nullptr };
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
			{ []() { return std::string("Score >"); }, []() { goToSubmenu(Page::SCORE); }, nullptr },
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

	auto saveScoreData = []() {
		if (ScoreManager::getCurrentScore()) {
			ScoreManager::save("score.dat");
		}
	};

	menuPages[Page::SCORE] = {
		"Debug Menu / Score",
		{
			{ []() { return std::string("PSCD (Player Stage Clear Data) >"); }, []() { goToSubmenu(Page::SCORE_PSCD); }, nullptr },
			{ []() { return std::string("CLRD (Clear Data) >"); }, []() { goToSubmenu(Page::SCORE_CLRD); }, nullptr },
			{ []() { return std::string("HSCD (Highscore Data) >"); }, []() { goToSubmenu(Page::SCORE_HSCD); }, nullptr },
			{ []() { return std::string("SPCD (Spellcard Data) >"); }, []() { goToSubmenu(Page::SCORE_SPCD); }, nullptr },
			{ []() { return std::string("PSTD (Gameplay Data) >"); }, []() { goToSubmenu(Page::SCORE_PSTD); }, nullptr },
			{ []() { return std::string("Save Score Data Now"); }, saveScoreData, nullptr },
			{ []() { return std::string("Reset Score Data"); }, []() { if (ScoreManager::getCurrentScore()) { ScoreManager::getCurrentScore()->reset(); ScoreManager::save("score.dat"); } }, nullptr },
			createBackMenuItem()
		}
	};

	menuPages[Page::SCORE_PSCD] = {
		"Debug Menu / Score / PSCD",
		[]() -> std::vector<MenuItem> {
			std::vector<MenuItem> items;

			if (!ScoreManager::getCurrentScore()) {
				items.push_back({ []() { return std::string("(no score data)"); }, nullptr, nullptr });
			} else {
				for (int character = 0; character < CHARACTER_COUNT; ++character) {
					int characterCopy = character;
					items.push_back({
						[characterCopy]() -> std::string {
							return format("Character %d >", characterCopy + 1);
						},
						[characterCopy]() {
							gScoreMenuContext.character = characterCopy;
							goToSubmenu(Page::SCORE_PSCD_DIFFICULTY);
						},
						nullptr
					});
				}
			}

			items.push_back(createBackMenuItem());
			return items;
		}()
	};

	menuPages[Page::SCORE_PSCD_DIFFICULTY] = {
		"Debug Menu / Score / PSCD / Difficulty",
		[]() -> std::vector<MenuItem> {
			std::vector<MenuItem> items;

			if (!ScoreManager::getCurrentScore()) {
				items.push_back({ []() { return std::string("(no score data)"); }, nullptr, nullptr });
			} else {
				for (int difficulty = 0; difficulty < DIFFICULTY_COUNT; ++difficulty) {
					int difficultyCopy = difficulty;
					items.push_back({
						[difficultyCopy]() -> std::string {
							return format("%s >", getDifficultyLabel(static_cast<uint8_t>(difficultyCopy)));
						},
						[difficultyCopy]() {
							gScoreMenuContext.difficulty = difficultyCopy;
							goToSubmenu(Page::SCORE_PSCD_STAGE);
						},
						nullptr
					});
				}
			}

			items.push_back(createBackMenuItem());
			return items;
		}()
	};

	menuPages[Page::SCORE_PSCD_STAGE] = {
		"Debug Menu / Score / PSCD / Stage",
		[]() -> std::vector<MenuItem> {
			std::vector<MenuItem> items;

			const Score* score = ScoreManager::getCurrentScore();
			if (!score) {
				items.push_back({ []() { return std::string("(no score data)"); }, nullptr, nullptr });
			} else {
				const int character = gScoreMenuContext.character;
				const int difficulty = gScoreMenuContext.difficulty;

				for (int stage = 0; stage < STAGE_COUNT; ++stage) {
					int stageCopy = stage;
					items.push_back({
						[character, difficulty, stageCopy]() -> std::string {
							const Score* currentScore = ScoreManager::getCurrentScore();
							if (!currentScore) {
								return "(null)";
							}

							const PSCD& entry = currentScore->playerStageClearData[character][difficulty][stageCopy];
							return format("%-8s  %u", getStageLabel(static_cast<uint8_t>(stageCopy)), entry.score);
						},
						nullptr,
						[character, difficulty, stageCopy](int direction) {
							Score* currentScore = ScoreManager::getCurrentScore();
							if (!currentScore) {
								return;
							}

							PSCD& entry = currentScore->playerStageClearData[character][difficulty][stageCopy];
							const int64_t step = isFineAdjustHeld() ? 1000 : 10000;
							const int64_t next = static_cast<int64_t>(entry.score) + (static_cast<int64_t>(direction) * step);
							entry.score = static_cast<uint32_t>(clamp(next, static_cast<int64_t>(0), static_cast<int64_t>(0xFFFFFFFFu)));
						}
					});
				}
			}

			items.push_back(createBackMenuItem());
			return items;
		}()
	};

	menuPages[Page::SCORE_CLRD] = {
		"Debug Menu / Score / CLRD",
		[]() -> std::vector<MenuItem> {
			std::vector<MenuItem> items;

			if (!ScoreManager::getCurrentScore()) {
				items.push_back({ []() { return std::string("(no score data)"); }, nullptr, nullptr });
			} else {
				for (int character = 0; character < CHARACTER_COUNT; ++character) {
					int characterCopy = character;
					items.push_back({
						[characterCopy]() -> std::string {
							return format("Character %d >", characterCopy + 1);
						},
						[characterCopy]() {
							gScoreMenuContext.character = characterCopy;
							goToSubmenu(Page::SCORE_CLRD_MODE);
						},
						nullptr
					});
				}
			}

			items.push_back(createBackMenuItem());
			return items;
		}()
	};

	menuPages[Page::SCORE_CLRD_MODE] = {
		"Debug Menu / Score / CLRD / Mode",
		[]() -> std::vector<MenuItem> {
			std::vector<MenuItem> items;

			if (!ScoreManager::getCurrentScore()) {
				items.push_back({ []() { return std::string("(no score data)"); }, nullptr, nullptr });
			} else {
				items.push_back({
					[]() -> std::string { return std::string("No Continues >"); },
					[]() {
						gScoreMenuContext.mode = 0;
						goToSubmenu(Page::SCORE_CLRD_DIFFICULTY);
					},
					nullptr
				});

				items.push_back({
					[]() -> std::string { return std::string("With Continues >"); },
					[]() {
						gScoreMenuContext.mode = 1;
						goToSubmenu(Page::SCORE_CLRD_DIFFICULTY);
					},
					nullptr
				});
			}

			items.push_back(createBackMenuItem());
			return items;
		}()
	};

	menuPages[Page::SCORE_CLRD_DIFFICULTY] = {
		"Debug Menu / Score / CLRD / Difficulty",
		[]() -> std::vector<MenuItem> {
			std::vector<MenuItem> items;

			const Score* score = ScoreManager::getCurrentScore();
			if (!score) {
				items.push_back({ []() { return std::string("(no score data)"); }, nullptr, nullptr });
			} else {
				const int character = gScoreMenuContext.character;
				const bool withContinues = (gScoreMenuContext.mode != 0);

				for (int difficulty = 0; difficulty < DIFFICULTY_COUNT; ++difficulty) {
					int difficultyCopy = difficulty;
					items.push_back({
						[character, difficultyCopy, withContinues]() -> std::string {
							const Score* currentScore = ScoreManager::getCurrentScore();
							if (!currentScore) {
								return "(null)";
							}

							const CLRD& entry = currentScore->clearData[character];
							const uint8_t value = withContinues ? entry.stageClearedWithContinues[difficultyCopy] : entry.stageCleared[difficultyCopy];
							return format("%-8s  %s", getDifficultyLabel(static_cast<uint8_t>(difficultyCopy)), getStageLabel(value));
						},
						nullptr,
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
					});
				}
			}

			items.push_back(createBackMenuItem());
			return items;
		}()
	};

	menuPages[Page::SCORE_HSCD] = {
		"Debug Menu / Score / HSCD",
		[]() -> std::vector<MenuItem> {
			std::vector<MenuItem> items;

			if (!ScoreManager::getCurrentScore()) {
				items.push_back({ []() { return std::string("(no score data)"); }, nullptr, nullptr });
			} else {
				for (int character = 0; character < CHARACTER_COUNT; ++character) {
					int characterCopy = character;
					items.push_back({
						[characterCopy]() -> std::string {
							return format("Character %d >", characterCopy + 1);
						},
						[characterCopy]() {
							gScoreMenuContext.character = characterCopy;
							goToSubmenu(Page::SCORE_HSCD_DIFFICULTY);
						},
						nullptr
					});
				}
			}

			items.push_back(createBackMenuItem());
			return items;
		}()
	};

	menuPages[Page::SCORE_HSCD_DIFFICULTY] = {
		"Debug Menu / Score / HSCD / Difficulty",
		[]() -> std::vector<MenuItem> {
			std::vector<MenuItem> items;

			if (!ScoreManager::getCurrentScore()) {
				items.push_back({ []() { return std::string("(no score data)"); }, nullptr, nullptr });
			} else {
				for (int difficulty = 0; difficulty < DIFFICULTY_COUNT; ++difficulty) {
					int difficultyCopy = difficulty;
					items.push_back({
						[difficultyCopy]() -> std::string {
							return format("%s >", getDifficultyLabel(static_cast<uint8_t>(difficultyCopy)));
						},
						[difficultyCopy]() {
							gScoreMenuContext.difficulty = difficultyCopy;
							goToSubmenu(Page::SCORE_HSCD_RANK);
						},
						nullptr
					});
				}
			}

			items.push_back(createBackMenuItem());
			return items;
		}()
	};

	menuPages[Page::SCORE_HSCD_RANK] = {
		"Debug Menu / Score / HSCD / Rank",
		[]() -> std::vector<MenuItem> {
			std::vector<MenuItem> items;

			const Score* score = ScoreManager::getCurrentScore();
			if (!score) {
				items.push_back({ []() { return std::string("(no score data)"); }, nullptr, nullptr });
			} else {
				const int character = gScoreMenuContext.character;
				const int difficulty = gScoreMenuContext.difficulty;

				for (int rank = 0; rank < HIGHSCORE_COUNT; ++rank) {
					int rankCopy = rank;
					items.push_back({
						[character, difficulty, rankCopy]() -> std::string {
							const Score* currentScore = ScoreManager::getCurrentScore();
							if (!currentScore) {
								return "(null)";
							}

							const HSCD& entry = currentScore->highscores[character][difficulty][rankCopy];
							char name[9] = {};
							memcpy(name, entry.name, 8);

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
						nullptr,
						[character, difficulty, rankCopy](int direction) {
							Score* currentScore = ScoreManager::getCurrentScore();
							if (!currentScore) {
								return;
							}

							HSCD& entry = currentScore->highscores[character][difficulty][rankCopy];
							const int64_t step = isFineAdjustHeld() ? 1000 : 10000;
							const int64_t next = static_cast<int64_t>(entry.score) + (static_cast<int64_t>(direction) * step);
							entry.score = static_cast<uint32_t>(clamp(next, static_cast<int64_t>(0), static_cast<int64_t>(0xFFFFFFFFu)));
							entry.state = static_cast<uint8_t>(HighscoreState::USER);
						}
					});
				}
			}

			items.push_back(createBackMenuItem());
			return items;
		}()
	};

	menuPages[Page::SCORE_SPCD] = {
		"Debug Menu / Score / SPCD",
		[]() -> std::vector<MenuItem> {
			std::vector<MenuItem> items;

			const Score* score = ScoreManager::getCurrentScore();
			if (!score) {
				items.push_back({ []() { return std::string("(no score data)"); }, nullptr, nullptr });
			} else {
				for (int spellcard = 0; spellcard < SPELLCARD_COUNT; ++spellcard) {
					int spellcardCopy = spellcard;
					items.push_back({
						[spellcardCopy]() -> std::string {
							const Score* currentScore = ScoreManager::getCurrentScore();
							if (!currentScore) {
								return "(null)";
							}

							const SPCD& sp = currentScore->spellcards[spellcardCopy];
							char name[65] = {};
							memcpy(name, sp.name, 64);

							const float rate = (sp.attempts > 0)
								? (static_cast<float>(sp.success) / static_cast<float>(sp.attempts) * 100.0f)
								: 0.0f;

							return format("#%-3d  %-20s  %u/%u (%.0f%%)  score:%u",
								spellcardCopy + 1,
								name,
								static_cast<unsigned>(sp.success),
								static_cast<unsigned>(sp.attempts),
								rate,
								sp.score);
						},
						nullptr,
						[spellcardCopy](int direction) {
							Score* currentScore = ScoreManager::getCurrentScore();
							if (!currentScore) {
								return;
							}

							SPCD& sp = currentScore->spellcards[spellcardCopy];
							const int64_t step = isFineAdjustHeld() ? 1000 : 10000;
							const int64_t next = static_cast<int64_t>(sp.score) + (static_cast<int64_t>(direction) * step);
							sp.score = static_cast<uint32_t>(clamp(next, static_cast<int64_t>(0), static_cast<int64_t>(0xFFFFFFFFu)));
						}
					});
				}
			}

			items.push_back(createBackMenuItem());
			return items;
		}()
	};

	menuPages[Page::SCORE_PSTD] = {
		"Debug Menu / Score / PSTD",
		[]() -> std::vector<MenuItem> {
			std::vector<MenuItem> items;

			const Score* score = ScoreManager::getCurrentScore();
			if (!score) {
				items.push_back({ []() { return std::string("(no score data)"); }, nullptr, nullptr });
			} else {
				items.push_back({
					[]() -> std::string {
						const Score* currentScore = ScoreManager::getCurrentScore();
						if (!currentScore) {
							return "(null)";
						}

						const PSTD& ps = currentScore->playStats;
						const uint32_t totalSec = ps.timePlayed.seconds;
						const uint32_t gameSec = ps.timePlayedGame.seconds;
						const uint32_t th = totalSec / 3600, tm = (totalSec % 3600) / 60, ts = totalSec % 60;
						const uint32_t gh = gameSec / 3600, gm = (gameSec % 3600) / 60, gs = gameSec % 60;
						return format("Total time:  %02u:%02u:%02u  (in-game: %02u:%02u:%02u)", th, tm, ts, gh, gm, gs);
					},
					nullptr,
					nullptr
				});

				const char* diffLabels[] = { "Easy", "Normal", "Hard", "Illusory", "Extra" };
				for (int difficulty = 0; difficulty < DIFFICULTY_COUNT; ++difficulty) {
					int difficultyCopy = difficulty;
					items.push_back({
						[difficultyCopy, diffLabels]() -> std::string {
							const Score* currentScore = ScoreManager::getCurrentScore();
							if (!currentScore) {
								return "(null)";
							}

							const GPLD& g = currentScore->playStats.dataDifficulty[difficultyCopy];
							uint32_t totalPlays = 0;
							for (int character = 0; character < CHARACTER_COUNT; ++character) {
								totalPlays += g.plays[character];
							}

							return format("%-8s  plays:%u  clears:%u  cont:%u  prac:%u",
								diffLabels[difficultyCopy],
								totalPlays,
								g.clears,
								g.continues,
								g.practices);
						},
						nullptr,
						nullptr
					});
				}

				items.push_back({
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

						return format("TOTAL     plays:%u  clears:%u  cont:%u  prac:%u",
							totalPlays,
							g.clears,
							g.continues,
							g.practices);
					},
					nullptr,
					nullptr
				});
			}

			items.push_back(createBackMenuItem());
			return items;
		}()
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
	static std::string dynamicTitle;

	switch (page) {
	case Page::SCORE_PSCD_DIFFICULTY:
		dynamicTitle = format("Debug Menu / Score / PSCD / Character %d", gScoreMenuContext.character + 1);
		return dynamicTitle.c_str();
	case Page::SCORE_PSCD_STAGE:
		dynamicTitle = format("Debug Menu / Score / PSCD / Character %d / %s", gScoreMenuContext.character + 1, getDifficultyLabel(static_cast<uint8_t>(gScoreMenuContext.difficulty)));
		return dynamicTitle.c_str();
	case Page::SCORE_CLRD_MODE:
		dynamicTitle = format("Debug Menu / Score / CLRD / Character %d", gScoreMenuContext.character + 1);
		return dynamicTitle.c_str();
	case Page::SCORE_CLRD_DIFFICULTY:
		dynamicTitle = format("Debug Menu / Score / CLRD / Character %d / %s", gScoreMenuContext.character + 1, gScoreMenuContext.mode == 0 ? "No Continues" : "With Continues");
		return dynamicTitle.c_str();
	case Page::SCORE_HSCD_DIFFICULTY:
		dynamicTitle = format("Debug Menu / Score / HSCD / Character %d", gScoreMenuContext.character + 1);
		return dynamicTitle.c_str();
	case Page::SCORE_HSCD_RANK:
		dynamicTitle = format("Debug Menu / Score / HSCD / Character %d / %s", gScoreMenuContext.character + 1, getDifficultyLabel(static_cast<uint8_t>(gScoreMenuContext.difficulty)));
		return dynamicTitle.c_str();
	default:
		break;
	}

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
