#include "Scene/Game.h"
#include "Engine/Supervisor.h"

void Game::init() {
    gSupervisor.isInGame = true;
    // Strategy here is to load all necessary resources common to every stage, etc...
}

void Game::destroy() {
    gSupervisor.isInGame = false;
}

void Game::update() {

}

void Game::render() {
    ClearDrawScreen();
}

void Game::restore() {
}