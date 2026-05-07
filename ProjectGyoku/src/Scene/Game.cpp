#include "Scene/Game.h"
#include "Engine/Supervisor.h"
#include "Engine/Score.h"
#include "Engine/Utils.h"
#include "Engine/Math/FPS.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Format/SHT.h"
#include "Engine/Log.h"
#include "Engine/Utils.h"

void Game::init() {
    BGMPlayer::stop();
    ScoreManager::save("score.dat");
    gSupervisor.isInGame = true;

    this->createSurfaces();

	gGameManager.gameSpeed = 1.0f;

    gGameManager.frame = 0;
    gGameManager.shownScore = gGameManager.score = 0;
    
    if(gGameManager.inPracticeMode){
        gGameManager.highscore = ScoreManager::getCurrentScore()->getStageHighscore(gGameManager.character, gGameManager.difficulty, gGameManager.stage);
    } else {
        gGameManager.highscore = ScoreManager::getCurrentScore()->getHighscore(gGameManager.character, gGameManager.difficulty);
    }

    this->playerAnimation = ANMManager::load(format("player%02d", gGameManager.character / SHOT_TYPE_COUNT));
    this->playerSHT = SHT::load(format("data/player/player%02d%c.sht", gGameManager.character / SHOT_TYPE_COUNT, gGameManager.character % SHOT_TYPE_COUNT + 'a'));

    gGameManager.startingLives = 2;
    gGameManager.startingBombs = this->playerSHT.bombs;

    this->setDefaultPlayerValues();
    gGameManager.deaths = 0;
    gGameManager.continuesUsed = 0;
    gGameManager.bombsUsed = 0;
    gGameManager.pointsTotal = 0;
    gGameManager.grazeTotal = 0;
    gGameManager.spellcardsCaptured = 0;

    gGameManager.enemyNextItem = 0;
    gGameManager.enemyDeathCount = 0;

    gGameManager.isTimeStopped = false;
    gGameManager.demoFrames = 0;

    this->state = GameState::RUNNING;
    this->transitionFrame = 0;

    // Strategy here is to load all necessary resources common to every stage, etc...
    this->hud = std::make_shared<Interface>(this);

    this->startStage();
}

void Game::createSurfaces() {
    if (gGameManager.gameSurface) { DeleteGraph(gGameManager.gameSurface); }
	gGameManager.gameSurface = MakeScreen(GAME_WIDTH, GAME_HEIGHT);

    if (gGameManager.gameInterfaceSurface) { DeleteGraph(gGameManager.gameInterfaceSurface); }
    gGameManager.gameInterfaceSurface = MakeScreen(WINDOW_WIDTH, WINDOW_HEIGHT, TRUE);

    if (this->finalSurface) { DeleteGraph(this->finalSurface); }
    this->finalSurface = MakeScreen(GAME_WIDTH, GAME_HEIGHT);
}

void Game::destroy() {
    ScoreManager::save("score.dat");
    gSupervisor.isInGame = false;

    if (gGameManager.gameSurface) {
		DeleteGraph(gGameManager.gameSurface);
		gGameManager.gameSurface = -1;
	}
    if (this->finalSurface) {
        DeleteGraph(this->finalSurface);
        this->finalSurface = -1;
    }
}

void Game::setDefaultPlayerValues() {
    gGameManager.lives = gGameManager.startingLives;
    gGameManager.bombs = gGameManager.startingBombs;
    gGameManager.power = 0;
    gGameManager.powerBonus = 0;
    gGameManager.points = 0;
    gGameManager.graze = 0;
    // maybe enemynextitem/enemydeathcount too?
}

void Game::startStage()
{
    // 1. Initialize Stage Data
    gGameManager.randomSeed = gRng.getUInt();
    gGameManager.rng.setSeed(gGameManager.randomSeed);
    gGameManager.gameSpeed = 1.0f;

    gGameManager.shownScore = gGameManager.score;
    gGameManager.points = 0;
    gGameManager.graze = 0;

    // 2. Initialize Player
    this->player = std::make_shared<Player>(this, this->playerAnimation, this->playerSHT);

    // 3. Load Stage Resources (enemies, boss, etc...)
    
    // 3a. Enemy ANM

    // 3b. Stage ECL

    // 3c. Stage MSG

    // 3d. Stage STD

    // 4. Interface/Background
    this->hud->startStage();

    // 5. Start Replay Manager
}

void Game::update() {
    if(this->state == GameState::RUNNING) {
        this->updateGame();
    } else {
        switch(this->state) {
            case GameState::PAUSE_START:
                if (this->transitionFrame++ >= 30) {
                    this->transitionFrame = 30;
                    this->state = GameState::PAUSED;
                }
                break;
            case GameState::CONTINUE_START:
                if (this->transitionFrame++ >= 30) {
                    this->transitionFrame = 30;
                    this->state = GameState::CONTINUE;
                }
                break;
            case GameState::PAUSE_END:
                if (this->transitionFrame-- <= 0) {
                    this->transitionFrame = 0;
                    this->state = GameState::RUNNING;
                }
                break;
            case GameState::CONTINUE_END:
                if (this->transitionFrame-- <= 0) {
                    this->transitionFrame = 0;
                    this->state = GameState::RUNNING;
                }
                break;
            case GameState::PAUSED:
                this->updatePause();
                break;
            case GameState::CONTINUE:
                this->updateContinue();
                break;
            default:
                break;
        }
    }
    if(this->hud) { this->hud->update(); }
}

void Game::updateGame() {
    // --- Backup Inputs ---
    // --- Handle Replay ---
    // --- Update ECL Runners ---
    // --- Filter Removed Objects ---
    // --- Update Everything ---
    // -- Background --
    // -- MSG Runners --
    // -- Player --
    this->updatePlayer();
    // -- Enemies --
    // -- Effects --
    // -- Bullets --
    // -- Lasers --
    // -- Interface --
    if (this->hud) { this->hud->updateGame(); }
    // -- Labels --
    // -- Faces --
    // --- Cleanup ---
    // --- Restore Inputs ---

    if (Input::gameInputPressed[static_cast<uint8_t>(GameInput::PAUSE)]) {
        SFXPlayer::play(SFX::PAUSE);
        this->state = GameState::PAUSE_START;
        this->transitionFrame = 0;
        FPS::setFPS(60);
    }

    gGameManager.frame++;
}

void Game::updatePlayer() {
    if(gGameManager.isTimeStopped) { return; }

    // Update Player
    if(this->player) { this->player->update(); }

    uint32_t oldShownScore = gGameManager.shownScore;
    if (gGameManager.shownScore != gGameManager.score) {
        uint32_t scoreDiff = (gGameManager.score - gGameManager.shownScore) / 32;
        scoreDiff = clamp(scoreDiff, 10U, 78910U);
        scoreDiff = (scoreDiff / 10) * 10; // round down to nearest 10

        gGameManager.shownScore += scoreDiff;
        gGameManager.shownScore = min(gGameManager.shownScore, gGameManager.score);

        if (gGameManager.stage < static_cast<uint8_t>(Stage::EXTRA)) {
            for (uint32_t i : {10000000, 20000000, 40000000, 60000000}) {
                if (gGameManager.shownScore >= i && oldShownScore < i) {
                    this->extendPlayer();
                }
            }
        }
    }
}

void Game::updatePause() {
    if (Input::gameInputPressed[static_cast<uint8_t>(GameInput::PAUSE)]) {
        this->state = GameState::PAUSE_END;
        this->transitionFrame = 30;
    }

    // Restart the game
    if (Input::inputPressed[KEY_INPUT_R]) {
        gStateManager.setState(std::make_shared<Game>());
    }
}

void Game::updateContinue() {
    // very barebones for now, but FIRE to continue else BOMB to result screen
    if (Input::gameInputPressed[static_cast<uint8_t>(GameInput::FIRE)]) {
        gGameManager.continuesUsed++;
        gGameManager.shownScore = gGameManager.continuesUsed;
        
        this->setDefaultPlayerValues();
        this->state = GameState::CONTINUE_END;
        this->transitionFrame = 30;
    } else if (Input::gameInputPressed[static_cast<uint8_t>(GameInput::BOMB)]) {
        Log::error("Game::updateContinue() - Player chose to end the game, add Result Screen!");
    }
}

void Game::render() {
    ClearDrawScreen();

    // --- Game Interface Surface ---
    SetDrawScreen(gGameManager.gameInterfaceSurface);
    ClearDrawScreen();

    // -- Interface --
    if(this->hud) { this->hud->renderGame(); }

    // --- Game Surfance ---
    SetDrawScreen(gGameManager.gameSurface);

    DrawBox(0, 0, GAME_WIDTH, GAME_HEIGHT, GetColor(0, 0, 0), TRUE);

    // -- Background --
    // -- Enemies --
    // -- Effects --
    // -- Player Bullets --
    // -- Player Lasers --
    // -- Player --
    if(this->player) { this->player->render(); }
    // -- Bullets --
    // -- Lasers --
    // -- Laser Effects --
    // -- Cancelled Bullets --
    // -- Items --
    // -- Hitbox --
    if(this->player) { this->player->renderHitbox(); }
    // -- Labels --
    // -- MSG --

    // --- Final Surface ---
    SetDrawScreen(this->finalSurface);
    
    DrawGraph(0, 0, gGameManager.gameSurface, FALSE);
    DrawGraph(0, 0, gGameManager.gameInterfaceSurface, TRUE);

    if (this->state != GameState::RUNNING) {
        int blurAmount = (int)((this->transitionFrame / 30.0f) * 100.0f);
        GraphFilter(this->finalSurface, DX_GRAPH_FILTER_GAUSS, 8, blurAmount);
    }

    // --- Main Surface ---
    SetDrawScreen(DX_SCREEN_BACK);
    DrawBox(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GetColor(30, 30, 30), TRUE);

    // -- Game Surface --
    DrawGraph(GAME_REGION_X, GAME_REGION_Y, this->finalSurface, TRUE);

    // -- Interface --
    if(this->hud) { this->hud->render(); }
}

void Game::restore() {
    this->createSurfaces();
}

void Game::triggerContinueMenu() {
    this->state = GameState::CONTINUE_START;
    this->transitionFrame = 0;
}

void Game::extendPlayer() {
    if (gGameManager.lives < 8) { gGameManager.lives++; }
    SFXPlayer::play(SFX::EXTEND);
}