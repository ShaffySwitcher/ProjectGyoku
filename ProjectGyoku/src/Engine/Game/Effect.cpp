#include "Engine/Game/Effect.h"
#include "Engine/Graphics/Sprite.h"

Effect::Effect(Vector position, uint32_t id, std::shared_ptr<ANM> animation, uint32_t spriteOffset) {
    this->position = position;
    this->drawable = std::make_shared<Sprite>();
    this->runner = std::make_shared<ANMRunner>(animation, id, drawable, spriteOffset);
}

bool Effect::update() {
    if (this->runner) {
        if (!this->runner->step()) {
            this->removed = true;
        }
    }

    return !this->removed;
}

void Effect::interrupt(int32_t label, bool setVisible) {
    if (this->runner) {
        this->runner->interrupt(label, setVisible);
    }
}