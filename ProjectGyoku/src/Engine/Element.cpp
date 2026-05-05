#include "Engine/Element.h"

Element::Element(Vector position) {
    this->position = position;
    this->offset = Vector(0, 0, 0);

    this->drawable = nullptr;
    this->runner = nullptr;
    this->removed = false;
}

void Element::render() {
    if (drawable) {
        drawable->render(position + offset);
    }
}