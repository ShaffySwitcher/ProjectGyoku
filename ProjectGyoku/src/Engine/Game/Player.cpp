#include "Engine/Game/Player.h"
#include "Engine/Utils.h"
#include "Engine/Input.h"
#include "Engine/Supervisor.h"

Player::Player(std::shared_ptr<ANM> animation, const SHT &sht) : Element(Vector(GAME_WIDTH / 2.0f, GAME_WIDTH)) {
    this->animation = animation;
    this->sht = sht;
    this->game = nullptr;

	this->direction = PlayerDirection::PLAYER_DIRECTION_NONE;
    this->state = PlayerState::PLAYER_STATE_ALIVE;

    this->fireTimer = 0;
    this->deathTimer = 0;

    this->invTimer = TO_FRAMES(5);
    
    this->setAnim(PLAYER_ANIM_IDLE);

    // Initialize orbs
    this->orbs[0] = std::make_unique<Effect>(this->position, 10, animation);
    this->orbs[0]->setOffset(Vector(-24, 0));
    this->orbs[1] = std::make_unique<Effect>(this->position, 10, animation);
    this->orbs[1]->setOffset(Vector(24, 0));

    this->orbInterpolator = nullptr;

    // Initialize hitbox
    this->hitbox[0] = std::make_unique<Effect>(this->position, 11, animation);
    this->hitbox[1] = std::make_unique<Effect>(this->position, 12, animation);
    this->hitbox[2] = std::make_unique<Effect>(this->position, 13, animation);
}

void Player::update() {
    float dx = 0.0f;
    float dy = 0.0f;

    if(this->deathTimer == 0) {
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
            setAnim(PLAYER_ANIM_MOVE_LEFT);
        } else if(dx > 0.0f && direction != PlayerDirection::PLAYER_DIRECTION_RIGHT) {
            direction = PlayerDirection::PLAYER_DIRECTION_RIGHT;
            setAnim(PLAYER_ANIM_MOVE_RIGHT);
        } else if(dx == 0.0f && direction != PlayerDirection::PLAYER_DIRECTION_NONE) {
            if(direction == PlayerDirection::PLAYER_DIRECTION_LEFT) {
                setAnim(PLAYER_ANIM_MOVE_END_LEFT);
            } else if(direction == PlayerDirection::PLAYER_DIRECTION_RIGHT) {
                setAnim(PLAYER_ANIM_MOVE_END_RIGHT);
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

    // TO-DO: Bomb

    // TO-DO: Death animation

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