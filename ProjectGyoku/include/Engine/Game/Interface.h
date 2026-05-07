#pragma once

#include "Scene/Game.h"
#include <array>

class Game;

#define HUD_ANIM_SCRIPT_HIGHSCORE_LABEL 11
#define HUD_ANIM_SCRIPT_DISCLAIMER_TEXT 20

class Interface {
public:
    Interface(Game* game);

    void update();
    void updateGame();

    void render();
    void renderGame();

    void startStage();

private:
    Game* game;
    std::shared_ptr<ANM> hudAnimation;

    std::vector<std::shared_ptr<Effect>> elements;

    std::array<std::shared_ptr<Effect>, 8> lifeIcons;
    std::array<std::shared_ptr<Effect>, 8> bombIcons;

    std::shared_ptr<Effect> fullPowerLabel;

    uint8_t oldPower = 0;
    uint8_t oldLives = 0;
    uint8_t oldBombs = 0;

    void detectChanges();
    void renderPowerDisplay();
};