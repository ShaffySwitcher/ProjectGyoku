#pragma once

#include "Engine/Element.h"
#include "Engine/Graphics/Sprite.h"
#include "Engine/Format/SHT.h"
#include "Engine/Game/Effect.h"
#include "Scene/Game.h"
#include <array>

class Player;
class Game;

#define PLAYER_ANIM_SCRIPT_IDLE 0
#define PLAYER_ANIM_SCRIPT_MOVE_LEFT 1
#define PLAYER_ANIM_SCRIPT_MOVE_END_LEFT 2
#define PLAYER_ANIM_SCRIPT_MOVE_RIGHT 3
#define PLAYER_ANIM_SCRIPT_MOVE_END_RIGHT 4

enum class PlayerState {
	PLAYER_STATE_ALIVE,
	PLAYER_STATE_DYING,
	PLAYER_STATE_DEAD
};

enum class PlayerDirection {
	PLAYER_DIRECTION_LEFT = -1,
	PLAYER_DIRECTION_NONE,
	PLAYER_DIRECTION_RIGHT
};

struct PlayerBomb {
    void (Player::*initFunc)() = nullptr;
    void (Player::*destroyFunc)() = nullptr;
    void (Player::*updateFunc)() = nullptr;
    void (Player::*renderFunc)() = nullptr;

    void init(Player* p)    { if (initFunc)    (p->*initFunc)(); }
    void destroy(Player* p) { if (destroyFunc) (p->*destroyFunc)(); }
    void update(Player* p)  { if (updateFunc)  (p->*updateFunc)(); }
    void render(Player* p)  { if (renderFunc)  (p->*renderFunc)(); }
};

class Player : public Element {
public:
    Player(Game* game, std::shared_ptr<ANM> animation, const SHT& sht);

	void update();
	virtual void render() override;
	void renderHitbox();

	void collide();
private:
	Game* game;
	std::shared_ptr<ANM> animation;
	SHT sht;
	PlayerState state;
	PlayerDirection direction;

	Timer fireTimer;
	Timer bombTimer;
	Timer deathTimer;
	Timer invTimer;

	bool focused = false;

	std::array<std::unique_ptr<Effect>, 2> orbs;
	std::array<std::unique_ptr<Effect>, 3> hitbox;

	std::unique_ptr<Interpolator<Vector>> orbInterpolator;

	PlayerBomb bomb;

	void setAnim(uint32_t id);
	void startFocusing();
	void stopFocusing();
	void fire();

	// Bomb functions
	void lloydABombInit();
	void lloydABombDestroy();
	void lloydABombUpdate();
	void lloydABombRender();

	void lloydBBombInit();
	void lloydBBombDestroy();
	void lloydBBombUpdate();
	void lloydBBombRender();

	void lloydCBombInit();
	void lloydCBombDestroy();
	void lloydCBombUpdate();
	void lloydCBombRender();
};