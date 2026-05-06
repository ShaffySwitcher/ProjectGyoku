#pragma once

#include "Engine/Element.h"

class Effect : public Element {
public:
    Effect(Vector position, uint32_t id, std::shared_ptr<ANM> animation, uint32_t spriteOffset = 0);

    bool update();
    void interrupt(int32_t label, bool setVisible = false);
};