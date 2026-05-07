#include "Engine/Game/Interface.h"
#include "Engine/Supervisor.h"
#include "Engine/Utils.h"
#include "Engine/Log.h"
#include "Engine/Graphics/Text.h"

Interface::Interface(Game* game) {
    this->game = game;
    this->hudAnimation = ANMManager::load("hud");

    // Set up elements
    for (int i = 0; i < 15; i++) {
        this->elements.push_back(std::make_shared<Effect>(Vector(0, 32 * static_cast<float>(i)), 0, this->hudAnimation));
    }

    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 15; j++) {
            this->elements.push_back(std::make_shared<Effect>(Vector(416 + 32 * static_cast<float>(i), 32 * static_cast<float>(j)), 0, this->hudAnimation));
        }
    }

    for (int i = 0; i < 12; i++) {
        this->elements.push_back(std::make_shared<Effect>(Vector(32 + 32 * static_cast<float>(i), 0), 1, this->hudAnimation));
        this->elements.push_back(std::make_shared<Effect>(Vector(32 + 32 * static_cast<float>(i), 464), 2, this->hudAnimation));
    }

    // Set up labels
    this->elements.push_back(std::make_shared<Effect>(Vector(0, 0), 10, this->hudAnimation, gGameManager.difficulty)); // Difficulty
    
    for (int i = HUD_ANIM_SCRIPT_HIGHSCORE_LABEL; i <= HUD_ANIM_SCRIPT_DISCLAIMER_TEXT; i++) {
        this->elements.push_back(std::make_shared<Effect>(Vector(0, 0), i, this->hudAnimation));
    }

    this->fullPowerLabel = std::make_shared<Effect>(Vector(0.0f, 0.0f), 23, this->hudAnimation);

    // Fill life and bomb array
    for (int i = 0; i < 8; i++) {
        this->lifeIcons[i] = std::make_shared<Effect>(Vector(500 + 16 * static_cast<float>(i), 136), 21, this->hudAnimation);
        this->lifeIcons[i]->interrupt(i < gGameManager.lives ? 0 : 1);
        this->bombIcons[i] = std::make_shared<Effect>(Vector(500 + 16 * static_cast<float>(i), 160), 22, this->hudAnimation);
        this->bombIcons[i]->interrupt(i < gGameManager.bombs ? 0 : 1);
    }
}

void Interface::startStage() {
    // TO-DO: animations
}

void Interface::update() {
    this->detectChanges();

    // HUD elements
    for (const auto& element : this->elements) {
        if(element) { element->update(); }
    }

    this->fullPowerLabel->update();

    // Lifes & Bombs
    for (int i = 0; i < 8; i++) {
        if (this->lifeIcons[i]) { this->lifeIcons[i]->update(); }
        if (this->bombIcons[i]) { this->bombIcons[i]->update(); }
    }
}

void Interface::detectChanges() {
    if (this->oldLives != gGameManager.lives) {
        if(gGameManager.lives > this->oldLives) {
            for (int i = this->oldLives; i < gGameManager.lives; i++) {
                if (this->lifeIcons[i]) { this->lifeIcons[i]->interrupt(0); }
            }
        } else {
            for (int i = gGameManager.lives; i < this->oldLives; i++) {
                if (this->lifeIcons[i]) { this->lifeIcons[i]->interrupt(1); }
            }
        }
    }

    if (this->oldBombs != gGameManager.bombs) {
        if (gGameManager.bombs > this->oldBombs) {
            for (int i = this->oldBombs; i < gGameManager.bombs; i++) {
                if (this->bombIcons[i]) { this->bombIcons[i]->interrupt(0); }
            }
        } else {
            for (int i = gGameManager.bombs; i < this->oldBombs; i++) {
                if (this->bombIcons[i]) { this->bombIcons[i]->interrupt(1); }
            }
        }

        this->oldBombs = gGameManager.bombs;
    }

    if (this->oldPower != gGameManager.power) {
        if (gGameManager.power >= 128 && this->oldPower < 128) {
            this->fullPowerLabel->interrupt(0);
        }
        else if (gGameManager.power < 128 && this->oldPower >= 128) {
            this->fullPowerLabel->interrupt(1);
        }
    }

    this->oldBombs = gGameManager.bombs;
    this->oldLives = gGameManager.lives;
    this->oldPower = gGameManager.power;
}

void Interface::updateGame() {
}

void Interface::render() {
    // HUD elements
    for (const auto& element : this->elements) {
        if(element) { element->render(); }
    }

    this->renderPowerDisplay();

    // Lifes & Bombs
    for (int i = 0; i < 8; i++) {
        if (this->lifeIcons[i]) { this->lifeIcons[i]->render(); }
        if (this->bombIcons[i]) { this->bombIcons[i]->render(); }
    }

    // Score & Highscore
    Text highscoreText(GetVector(496.0f, 58.0f), format("%09d", (gGameManager.highscore > gGameManager.shownScore) ? gGameManager.highscore : gGameManager.shownScore));
    highscoreText.setColor(Color::getColor(180, 180, 180));
    highscoreText.render();

    Text scoreText(GetVector(496.0f, 82.0f), format("%09d", gGameManager.shownScore));
    scoreText.setColor(Color::getColor(240, 240, 240));
    scoreText.render();

    Text(GetVector(516.0f, 216.0f), std::to_string(gGameManager.points)).render();
    Text(GetVector(516.0f, 240.0f), std::to_string(gGameManager.graze)).render();
}

void Interface::renderGame() {
}

void Interface::renderPowerDisplay() {
    VERTEX2D vertices[6]{};

    vertices[0] = Vertex::createVertex2D(GetVector(488.0f, 192.0f), 1.0f, Color::getColor(224, 224, 255, 224), 0.0f, 0.0f);
    vertices[1] = Vertex::createVertex2D(GetVector(gGameManager.power + 488.0f, 192.0f), 1.0f, Color::getColor(224, 224, 255, 128), 1.0f, 0.0f);
    vertices[2] = Vertex::createVertex2D(GetVector(488.0f, 208.0f), 1.0f, Color::getColor(224, 224, 255, 224), 0.0f, 1.0f);
    vertices[3] = Vertex::createVertex2D(GetVector(gGameManager.power + 488.0f, 208.0f), 1.0f, Color::getColor(224, 224, 255, 128), 1.0f, 1.0f);
    vertices[4] = vertices[1];
    vertices[5] = vertices[2];
    DrawPrimitive2D(vertices, 6, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE);

    this->fullPowerLabel->render();

    if (gGameManager.power < 128) {
        Text(GetVector(492.0f, 192.0f), format("%03i/128", gGameManager.power)).render();
    }
}