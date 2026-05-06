#pragma once

#include "Engine/Graphics/Drawable.h"
#include "Engine/Graphics/Animation.h"

class Element {
public:
    Element(Vector position = Vector(0, 0, 0));
    virtual ~Element() = default;
    
    virtual void render();

    void setPosition(const Vector& position) { this->position = position; }
    void setOffset(const Vector& offset) { this->offset = offset; }
    void setRemoved(bool removed) { this->removed = removed; }

    Vector getPosition() const { return position; }
    Vector getOffset() const { return offset; }
    bool isRemoved() const { return removed; }

protected:
    std::shared_ptr<Drawable> drawable = nullptr;
    std::shared_ptr<ANMRunner> runner = nullptr;

    Vector position;
    Vector offset;
    bool removed = false;
};