#pragma once

#include <DxLib.h>
#include "Engine/State.h"
#include "Engine/Game/Player.h"
#include "Engine/Game/Interface.h"

class Player;
class Interface;

enum class GameState {
    RUNNING,
    PAUSE_START,
    PAUSED,
    PAUSE_END,
    CONTINUE_START,
    CONTINUE,
    CONTINUE_END,
};

class Game : public State
{
public:
    virtual void init() override;
    virtual void destroy() override;
    virtual void update() override;
    virtual void render() override;
    virtual void restore() override;

    std::shared_ptr<Player> getPlayer() { return this->player; }
    
    void extendPlayer();

    void triggerContinueMenu();

private:
    // Game Objects
    std::shared_ptr<Interface> hud;
    std::shared_ptr<Player> player;

    // Common Assets
    std::shared_ptr<ANM> playerAnimation;
    SHT playerSHT;

    // Stage Assets

    // State Management
    GameState state;
    int transitionFrame = 0;
    int finalSurface;

    void createSurfaces();
    void setDefaultPlayerValues();
    void startStage();

    void updateGame();
    
    void updatePlayer();

    void updatePause();
    void updateContinue();
};