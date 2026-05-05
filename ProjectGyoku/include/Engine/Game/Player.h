#pragma once

#include "Engine/Element.h"
#include "Engine/Graphics/Sprite.h"
#include "Engine/Game/Format/SHT.h"
#include "Scene/Game.h"

class Game;

#define PLAYER_ANIM_IDLE 0
#define PLAYER_ANIM_MOVE_LEFT 1
#define PLAYER_ANIM_MOVE_END_LEFT 2
#define PLAYER_ANIM_MOVE_RIGHT 3
#define PLAYER_ANIM_MOVE_END_RIGHT 4

enum class PlayerState {
	PLAYER_STATE_ALIVE,
	PLAYER_STATE_SPAWNING,
	PLAYER_STATE_DEAD,
	PLAYER_STATE_INVULNERABLE
};

enum class PlayerDirection {
	PLAYER_DIRECTION_LEFT = -1,
	PLAYER_DIRECTION_NONE,
	PLAYER_DIRECTION_RIGHT
};

class Player : public Element {
public:
    Player(std::shared_ptr<Animation> animation, const SHT& sht);

	virtual void update() override;
	virtual void render() override;

	void setGame(Game* game) { this->game = game; }
private:
	Game* game;
	std::shared_ptr<Animation> animation;
	SHT sht;
	PlayerState state;
	PlayerDirection direction;

	Timer fireTimer;
	Timer deathTimer;
	Timer invTimer;

	bool focused = false;

	void setAnim(uint32_t id);
	void fire();
};