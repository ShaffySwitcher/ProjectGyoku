#include "Engine/Game/Player.h"
#include "Engine/Utils.h"
#include "Engine/Input.h"
#include "Engine/Supervisor.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Log.h"

Player::Player(Game* game, std::shared_ptr<ANM> animation, const SHT &sht) : Element(Vector(GAME_WIDTH / 2.0f, GAME_WIDTH)) {
    this->animation = animation;
    this->sht = sht;
    this->game = game;

	this->direction = PlayerDirection::PLAYER_DIRECTION_NONE;
    this->state = PlayerState::PLAYER_STATE_ALIVE;

    this->fireTimer = 0;
    this->deathTimer = 0;

    this->invTimer = TO_FRAMES(5);
    
    this->setAnim(PLAYER_ANIM_SCRIPT_IDLE);

    // Initialize orbs
    this->orbs[0] = std::make_unique<Effect>(this->position, 10, animation);
    this->orbs[0]->setOffset(Vector(-24, 0));
    this->orbs[1] = std::make_unique<Effect>(this->position, 10, animation);
    this->orbs[1]->setOffset(Vector(24, 0));

    this->orbInterpolator = nullptr;

    // Initialize hitbox
    this->hitbox[0] = std::make_unique<Effect>(this->position, 11, animation);
    this->hitbox[0]->getDrawable()->setScale((this->sht.hitbox / 4.0f) * Vector(1.0f, 1.0f)); // rescales based on spritesheet size
    this->hitbox[1] = std::make_unique<Effect>(this->position, 12, animation);
    this->hitbox[2] = std::make_unique<Effect>(this->position, 13, animation);

    // Bind bomb functions
    switch(gGameManager.character / SHOT_TYPE_COUNT) {
        case static_cast<uint8_t>(Character::LLOYD):
            switch(gGameManager.character % SHOT_TYPE_COUNT) {
                case static_cast<uint8_t>(ShotType::SHOT_TYPE_A):
                    this->bomb = {
                        &Player::lloydABombInit,
                        &Player::lloydABombDestroy,
                        &Player::lloydABombUpdate,
                        &Player::lloydABombRender
                    };
                    break;
                case static_cast<uint8_t>(ShotType::SHOT_TYPE_B):
                    this->bomb = {
                        &Player::lloydBBombInit,
                        &Player::lloydBBombDestroy,
                        &Player::lloydBBombUpdate,
                        &Player::lloydBBombRender
                    };
                    break;
                case static_cast<uint8_t>(ShotType::SHOT_TYPE_C):
                    this->bomb = {
                        &Player::lloydCBombInit,
                        &Player::lloydCBombDestroy,
                        &Player::lloydCBombUpdate,
                        &Player::lloydCBombRender
                    };
                    break;
            }
            break;
    }
}

void Player::update() {
    float dx = 0.0f;
    float dy = 0.0f;

    if(this->state == PlayerState::PLAYER_STATE_ALIVE) {
        float speed = (this->focused) ? sht.playerSpeed.focus : sht.playerSpeed.normal;
        float diagSpeed = (this->focused) ? sht.playerSpeed.focusDiagonal : sht.playerSpeed.diagonal;

        if (Input::gameInputCurrent[static_cast<uint8_t>(GameInput::DOWN)] && Input::gameInputCurrent[static_cast<uint8_t>(GameInput::RIGHT)]) {
			dx = diagSpeed;
			dy = diagSpeed;
		}
		else if (Input::gameInputCurrent[static_cast<uint8_t>(GameInput::DOWN)] && Input::gameInputCurrent[static_cast<uint8_t>(GameInput::LEFT)]) {
			dx = -diagSpeed;
			dy = diagSpeed;
		}
		else if (Input::gameInputCurrent[static_cast<uint8_t>(GameInput::UP)] && Input::gameInputCurrent[static_cast<uint8_t>(GameInput::RIGHT)]) {
			dx = diagSpeed;
			dy = -diagSpeed;
		}
		else if (Input::gameInputCurrent[static_cast<uint8_t>(GameInput::UP)] && Input::gameInputCurrent[static_cast<uint8_t>(GameInput::LEFT)]) {
			dx = -diagSpeed;
			dy = -diagSpeed;
		}
		else if (Input::gameInputCurrent[static_cast<uint8_t>(GameInput::RIGHT)]) {
			dx = speed;
		}
		else if (Input::gameInputCurrent[static_cast<uint8_t>(GameInput::LEFT)]) {
			dx = -speed;
		}
		else if (Input::gameInputCurrent[static_cast<uint8_t>(GameInput::DOWN)]) {
			dy = speed;
		}
		else if (Input::gameInputCurrent[static_cast<uint8_t>(GameInput::UP)]) {
			dy = -speed;
		}

        if(dx < 0.0f && direction != PlayerDirection::PLAYER_DIRECTION_LEFT) {
            direction = PlayerDirection::PLAYER_DIRECTION_LEFT;
            setAnim(PLAYER_ANIM_SCRIPT_MOVE_LEFT);
        } else if(dx > 0.0f && direction != PlayerDirection::PLAYER_DIRECTION_RIGHT) {
            direction = PlayerDirection::PLAYER_DIRECTION_RIGHT;
            setAnim(PLAYER_ANIM_SCRIPT_MOVE_RIGHT);
        } else if(dx == 0.0f && direction != PlayerDirection::PLAYER_DIRECTION_NONE) {
            if(direction == PlayerDirection::PLAYER_DIRECTION_LEFT) {
                setAnim(PLAYER_ANIM_SCRIPT_MOVE_END_LEFT);
            } else if(direction == PlayerDirection::PLAYER_DIRECTION_RIGHT) {
                setAnim(PLAYER_ANIM_SCRIPT_MOVE_END_RIGHT);
            }
            direction = PlayerDirection::PLAYER_DIRECTION_NONE;
        }

        this->position.x += dx * gGameManager.gameSpeed;
        this->position.y += dy * gGameManager.gameSpeed;

        this->position.x = clamp(this->position.x, 8.0f, static_cast<float>(GAME_WIDTH) - 8.0f);
        this->position.y = clamp(this->position.y, 16.0f, static_cast<float>(GAME_HEIGHT) - 16.0f);

        if (!this->focused && Input::gameInputCurrent[static_cast<uint8_t>(GameInput::FOCUS)]) {
            this->startFocusing();
        }
        else if (this->focused && !Input::gameInputCurrent[static_cast<uint8_t>(GameInput::FOCUS)]) {
            this->stopFocusing();
        }

        if(this->invTimer > 0) {
            this->invTimer--;
            int m = this->invTimer.getFrame() % 8;
            if (m == 7 || this->invTimer == 0) {this->drawable->setColor(Color::getColor(255, 255, 255));}
            else if (m == 1) { this->drawable->setColor(Color::getColor(64, 64, 128)); }
        }

        if (Input::gameInputCurrent[static_cast<uint8_t>(GameInput::FIRE)] && this->fireTimer == 0) {
            this->fireTimer = 30;
        }

        if (this->fireTimer > 0) {
            this->fire();
            this->fireTimer--;
        }

        // Update orbs
        if (this->orbInterpolator) {
            this->orbInterpolator->update(gGameManager.frame);
            Vector offset = this->orbInterpolator->getValue();

            this->orbs[0]->setOffset(offset * Vector(-1, 1));
            this->orbs[1]->setOffset(offset);

            if (this->orbInterpolator->isFinished()) {
                this->orbInterpolator = nullptr;
            }
        }

        for (const auto& orb : this->orbs) {
            if(orb) {
                orb->update();
                orb->setPosition(this->position);
            }
        }
        
        // Update hitbox
        for (const auto& hitbox : this->hitbox) {
            if(hitbox) {
                hitbox->update();
                hitbox->setPosition(this->position);
            }
        }
    }

    // Handle (death)bombing
    if (this->state == PlayerState::PLAYER_STATE_ALIVE || (this->state == PlayerState::PLAYER_STATE_DYING && this->deathTimer < this->sht.deathBombWindow)) {
        if (Input::gameInputPressed[static_cast<uint8_t>(GameInput::BOMB)] && this->bombTimer == 0 && gGameManager.bombs > 0) {
            gGameManager.bombsUsed++;
            gGameManager.bombs--;
            this->state = PlayerState::PLAYER_STATE_ALIVE;
            this->deathTimer = 0;
            
            // TO-DO: Set spellcard bombed flag here
            // TO-DO: Collect all items

            this->bomb.init(this);
        }
    }

    if (this->state == PlayerState::PLAYER_STATE_DYING) {
        this->deathTimer++;
        if (this->deathTimer == this->sht.deathBombWindow) {
            this->state = PlayerState::PLAYER_STATE_DEAD;
            this->deathTimer = 0;
            // TO-DO: Set spellcard capture failure flag here

            gGameManager.power = max(gGameManager.power - 16, 0);
            gGameManager.bombs = gGameManager.startingBombs;
            // TO-DO: unset player lasers

            gGameManager.deaths++;
            gGameManager.lives--;
            if (gGameManager.lives < 0) {
                if (gGameManager.inPracticeMode) {
                    Log::error("Player::update() - Player died in practice mode, add Result Screen!");
                }
                if (gGameManager.inReplayMode) {
                    Log::error("Player::update() - Player died in replay mode, add Replay Menu!");
                }
                if (gGameManager.continuesUsed >= 3 || gGameManager.difficulty == static_cast<uint8_t>(Difficulty::EXTRA)) {
                    Log::error("Player::update() - Game over, add Result Screen!");
                }

                this->game->triggerContinueMenu();
                return;
            } else {
                // TO-DO: drop bonus
            }

            this->drawable->setScale(Point(0.75f, 1.5f));
            this->drawable->fade(26, 96);
            this->drawable->scaleTo(26, Point(0.0f, 2.5f));
        }
    } else if (this->state == PlayerState::PLAYER_STATE_DEAD) {
        this->deathTimer++;
        if (this->deathTimer == 25) {
            // TO-DO: Cancel all bullets

            this->position = Vector(GAME_WIDTH / 2.0f, GAME_WIDTH);
            this->direction = PlayerDirection::PLAYER_DIRECTION_NONE;
            this->setAnim(PLAYER_ANIM_SCRIPT_IDLE);

            this->drawable->setAlpha(128);
            this->drawable->setScale(Point(0.0f, 2.5f));
            this->drawable->fade(30, 255);
            this->drawable->scaleTo(30, Point(1.0f, 1.0f));   
        } else if (this->deathTimer == 55) {
            this->invTimer = TO_FRAMES(5);
            this->state = PlayerState::PLAYER_STATE_ALIVE;
            this->deathTimer = 0;
        }
    }

    // Bomb update
    if (this->bombTimer > 0) {
        this->bomb.update(this);
        this->bombTimer--;

        if(this->bombTimer == 0) {
            this->bomb.destroy(this);
        }
        // TO-DO: Unset bomb labels
    }

    if (this->runner) { this->runner->step(); }
}

void Player::render() {
    Element::render();

    for (const auto& orb : this->orbs) {
        if(orb) { orb->render(); }
    }
}

void Player::renderHitbox() {
    for (const auto& hitbox : this->hitbox) {
        if(hitbox) {
            hitbox->render();
        }
    }
}

void Player::collide() {
    if (this->state != PlayerState::PLAYER_STATE_ALIVE || this->invTimer > 0) {
        return;
    }

    this->state = PlayerState::PLAYER_STATE_DYING;
    this->deathTimer = 0;

    // TO-DO: Particles
    SFXPlayer::play(SFX::DEATH);
}

void Player::setAnim(uint32_t id) {
    this->drawable = std::make_shared<Sprite>();
    this->runner = std::make_shared<ANMRunner>(animation, id, drawable);
}

void Player::startFocusing() {
    this->focused = true;
    
    this->orbInterpolator = std::make_unique<Interpolator<Vector>>(Vector(24, 0), gGameManager.frame, Vector(10, -32), gGameManager.frame + 8, InterpolationMode::EASE_OUT_CIRC);
    for (const auto& orb : this->orbs) {
        if(orb) { orb->interrupt(0); }
    }

    for (const auto& hitbox : this->hitbox) {
        if(hitbox) { hitbox->interrupt(0); }
    }
}

void Player::stopFocusing() {
    this->focused = false;

    this->orbInterpolator = std::make_unique<Interpolator<Vector>>(Vector(10, -32), gGameManager.frame, Vector(24, 0), gGameManager.frame + 8, InterpolationMode::EASE_OUT_CIRC);
    for (const auto& orb : this->orbs) {
        if(orb) { orb->interrupt(1); }
    }
    
    for (const auto& hitbox : this->hitbox) {
        if(hitbox) { hitbox->interrupt(1); }
    }
}

void Player::fire() {
    // TO-DO: Add firing logic
}