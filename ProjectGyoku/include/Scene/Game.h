#pragma once

#include <DxLib.h>
#include "Engine/State.h"
#include "Engine/Game/Player.h"

class Player;

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

private:
    std::shared_ptr<Player> player;
    
    GameState state;
    int transitionFrame = 0;

    void startStage();

    void updateGame();
    
    void updatePlayer();

    void updatePause();
    void updateContinue();
};