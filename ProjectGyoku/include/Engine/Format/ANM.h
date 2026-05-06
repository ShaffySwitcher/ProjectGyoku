#pragma once

#include <vector>
#include <map>
#include "Engine/Math/GyokuMath.h"
#include "Engine/Graphics/Texture.h"

enum class ANMOpcode : uint8_t {
    NOP = 0,

    SET_SPRITE = 1,
    SET_RANDOM_SPRITE = 2,

    SET_OFFSET = 3,
    SET_SCALE = 4,
    SET_ROTATION = 5,

    SET_COLOR = 6,
    SET_ALPHA = 7,
    SET_VISIBILITY = 8,
    SET_BLEND_MODE = 9,

    SCROLL_TEXTURE_X = 10,
    SCROLL_TEXTURE_Y = 11,

    FLIP_X = 12,
    FLIP_Y = 13,
    ANCHOR_TOP_LEFT = 14,

    SET_OFFSET_SPEED = 15,
    SET_SCALE_SPEED = 16,
    SET_ROTATION_SPEED = 17,

    Z_WRITE_DISABLE = 18,

    STOP = 19,
    PAUSE = 20,
    JUMP = 21,

    FADE_TO = 22,
    MOVE_TO = 23,
    ROTATE_TO = 24,
    SCALE_TO = 25,

    INTERRUPT_LABEL = 26
};


#pragma pack(push, 1)
struct ANMHeader {
	char magic[4]{};
	uint16_t version{};
	uint32_t numSprites{};
	uint32_t numScripts{};
	uint16_t width{};
	uint16_t height{};
	uint32_t spriteTableOffset{};
	uint32_t scriptTableOffset{};
	uint32_t pathOffset{};
	uint32_t pathOffsetAlpha{};
};
#pragma pack(pop)

struct ANMInstruction {
    uint32_t offset{};
    uint16_t time{};
    ANMOpcode opcode{};
    std::vector<uint8_t> args{};

    template<typename T>
    T as() const {
        if (sizeof(T) != args.size()) {
            throw std::runtime_error("Invalid argument size for instruction");
        }

        T value;
        std::memcpy(&value, args.data(), sizeof(T));
        return value;
    }

    template<typename T>
    T get(size_t offset = 0) const {
        if (offset + sizeof(T) > args.size()) {
            throw std::out_of_range("Argument out of range");
        }

        T value;
        std::memcpy(&value, args.data() + offset, sizeof(T));
        return value;
    }

    template<typename T>
    void set(const T& value, size_t offset = 0)
    {
        if (offset + sizeof(T) > args.size()) {
            throw std::out_of_range("Argument out of range");
        }

        std::memcpy(args.data() + offset, &value, sizeof(T));
    }
};



struct ANMScript {
    std::vector<ANMInstruction> instructions{};
    std::map<int32_t, uint32_t> interrupts{};
};


struct ANM {
    std::map<int32_t, Rect> sprites{};
    std::map<int32_t, ANMScript> scripts{};
    std::string path{};
    uint32_t revision = 0;
    Texture texture;

	static std::shared_ptr<ANM> load(const std::string& path, bool loadTexture = true);
};