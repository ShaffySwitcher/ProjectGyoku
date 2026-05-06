#pragma once

#include "Engine/Element.h"
#include "Engine/Graphics/Sprite.h"
#include "Engine/Format/SHT.h"
#include "Engine/Game/Effect.h"
#include "Scene/Game.h"
#include <array>


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

class Player;

class Player : public Element {
public:
    Player(std::shared_ptr<ANM> animation, const SHT& sht);

	void update();
	virtual void render() override;
	void renderHitbox();

	void setGame(Game* game) { this->game = game; }
private:
	Game* game;
	std::shared_ptr<ANM> animation;
	SHT sht;
	PlayerState state;
	PlayerDirection direction;

	Timer fireTimer;
	Timer deathTimer;
	Timer invTimer;

	bool focused = false;

	std::array<std::unique_ptr<Effect>, 2> orbs;
	std::array<std::unique_ptr<Effect>, 3> hitbox;

	std::unique_ptr<Interpolator<Vector>> orbInterpolator;

	void setAnim(uint32_t id);
	void startFocusing();
	void stopFocusing();
	void fire();
};