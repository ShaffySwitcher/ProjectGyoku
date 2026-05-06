#pragma once

#include "Engine/Math/GyokuMath.h"
#include <unordered_map>

struct Shot {
    int16_t interval;
    int16_t delay;
    Vector position;
    Vector hitbox;
    float angle;
    float speed;
    uint16_t damage;
    uint8_t orb;
    uint8_t type;
    uint16_t sprite;
    uint16_t sfx;
};

struct PlayerSpeed {
    float normal;
    float diagonal;
    float focus;
    float focusDiagonal;
};

struct SHT {
    int16_t shotCount;
    uint8_t bombs;
    uint32_t deathBombWindow;
    float hitbox;
    float grazebox;
    float itemMagnetSpeed;
    float itemBox;
    float pocLine;
    PlayerSpeed playerSpeed;
    std::unordered_map<int, std::vector<Shot>> shots;

    static SHT load(const std::string& path);
};