#include "Scene/Game.h"
#include "Engine/Supervisor.h"
#include "Engine/Score.h"
#include "Engine/Math/FPS.h"
#include "Engine/Audio/Audio.h"

#include "Engine/Game/Format/SHT.h"

void Game::init() {
    BGMPlayer::stop();
    ScoreManager::save("score.dat");
    gSupervisor.isInGame = true;

    if (gGameManager.gameSurface) {
		DeleteGraph(gGameManager.gameSurface);
		gGameManager.gameSurface = -1;
	}
	gGameManager.gameSurface = MakeScreen(GAME_WIDTH, GAME_HEIGHT);
	gGameManager.gameSpeed = 1.0f;

    gGameManager.frame = 0;
    gGameManager.shownScore = gGameManager.score = 0;
    
    if(gGameManager.inPracticeMode){
        gGameManager.highscore = ScoreManager::getCurrentScore()->getStageHighscore(gGameManager.character, gGameManager.difficulty, gGameManager.stage);
    } else {
        gGameManager.highscore = ScoreManager::getCurrentScore()->getHighscore(gGameManager.character, gGameManager.difficulty);
    }

    gGameManager.lives = gGameManager.startingLives;
    gGameManager.bombs = gGameManager.startingBombs;
    gGameManager.deaths = 0;
    gGameManager.continuesUsed = 0;
    gGameManager.bombsUsed = 0;
    gGameManager.power = 0;
    gGameManager.powerBonus = 0;
    gGameManager.points = 0;
    gGameManager.pointsTotal = 0;
    gGameManager.graze = 0;
    gGameManager.grazeTotal = 0;
    gGameManager.spellcardsCaptured = 0;

    gGameManager.isTimeStopped = false;
    gGameManager.demoFrames = 0;

    gGameManager.enemyNextItem = 0;
    gGameManager.enemyDeathCount = 0;

    this->state = GameState::RUNNING;
    this->transitionFrame = 0;

    // Strategy here is to load all necessary resources common to every stage, etc...

    this->startStage();
}

void Game::destroy() {
    ScoreManager::save("score.dat");
    gSupervisor.isInGame = false;

    if (gGameManager.gameSurface) {
		DeleteGraph(gGameManager.gameSurface);
		gGameManager.gameSurface = -1;
	}
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
    this->player = std::make_shared<Player>(ANMManager::load("player00"), SHT::load("data/player/player00.sht"));
    this->player->setGame(this);

    // 3. Load Stage Resources (enemies, boss, etc...)
    
    // 3a. Enemy ANM

    // 3b. Stage ECL

    // 3c. Stage MSG

    // 3d. Stage STD

    // 4. Interface Setup

    // 5. Start Replay Manager
}

void Game::update() {
    if(this->state == GameState::RUNNING) {
        this->updateGame();
    } else {
        switch(this->state) {
            case GameState::PAUSE_START:
                if (this->transitionFrame++ >= 30) {
                    this->transitionFrame = 0;
                    this->state = GameState::PAUSED;
                }
                break;
            case GameState::CONTINUE_START:
                if (this->transitionFrame++ >= 30) {
                    this->transitionFrame = 0;
                    this->state = GameState::CONTINUE;
                }
                break;
            case GameState::PAUSE_END:
                if (this->transitionFrame++ >= 30) {
                    this->transitionFrame = 0;
                    this->state = GameState::RUNNING;
                }
                break;
            case GameState::CONTINUE_END:
                if (this->transitionFrame++ >= 30) {
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
    if(this->player) {
        this->player->update();
    }
}

void Game::updatePause() {
    if (Input::gameInputPressed[static_cast<uint8_t>(GameInput::PAUSE)]) {
        this->state = GameState::PAUSE_END;
        this->transitionFrame = 0;
    }
}

void Game::updateContinue() {
}

void Game::render() {
    ClearDrawScreen();

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
    // -- Labels --
    // -- MSG --
    // -- Interface --

    // --- Main Surface ---
    SetDrawScreen(DX_SCREEN_BACK);

    DrawGraph(GAME_REGION_X, GAME_REGION_Y, gGameManager.gameSurface, FALSE);
}

void Game::restore() {
    if (gGameManager.gameSurface) { DeleteGraph(gGameManager.gameSurface); }
	gGameManager.gameSurface = MakeScreen(GAME_WIDTH, GAME_HEIGHT);
}