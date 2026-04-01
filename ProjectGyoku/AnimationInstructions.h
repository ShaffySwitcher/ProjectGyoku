#pragma once

#include <cstdint>

#pragma pack(push, 1)

struct SetSpriteArgs {
    int32_t id;
};

struct SetRandomSpriteArgs {
    int32_t min_id;
    int32_t amp;
};

struct SetOffsetArgs {
    float ox, oy, oz;
};

struct SetScaleArgs {
    float sx, sy;
};

struct SetRotationArgs {
    float rx, ry, rz;
};

struct SetColorArgs {
    uint8_t r, g, b;
};

struct SetAlphaArgs {
    uint8_t alpha;
};

struct SetVisibilityArgs {
    uint8_t visible;
};

struct SetBlendModeArgs {
    uint8_t mode;
};

struct ScrollTextureXArgs {
    float dx;
};

struct ScrollTextureYArgs {
    float dy;
};

struct SetOffsetSpeedArgs {
    float osx, osy, osz;
};

struct SetScaleSpeedArgs {
    float ssx, ssy;
};

struct SetRotationSpeedArgs {
    float srx, sry, srz;
};

struct ZWriteDisableArgs {
    uint8_t enabled;
};

struct JumpArgs {
    uint32_t offset;
};

struct InterruptLabelArgs {
    int32_t label;
};

struct FadeArgs {
    uint32_t alpha;
    uint32_t duration;
    uint8_t  mode;
};

struct MoveToArgs {
    float x, y, z;
    uint32_t duration;
    uint8_t  mode;
};

struct RotateToArgs {
    float rx, ry, rz;
    uint32_t duration;
    uint8_t  mode;
};

struct ScaleToArgs {
    float sx, sy;
    uint32_t duration;
    uint8_t  mode;
};

#pragma pack(pop)