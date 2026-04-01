#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include "Global.h"
#include "Texture.h"
#include "Drawable.h"
#include "AnimationInstructions.h"

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


struct Animation {
    std::map<int32_t, Rect> sprites{};
    std::map<int32_t, ANMScript> scripts{};
    std::string path{};
    Texture texture;

	static std::shared_ptr<Animation> loadFromFile(const std::string& path);
};

class ANMManager
{
public:
    static std::shared_ptr<Animation> load(const std::string& name, const std::string& path = "");
	static void unload(const std::string& name);

    static void restore();

private:
	static std::map<std::string, std::shared_ptr<Animation>> animations;
};

class ANMRunner
{
public:
	ANMRunner(std::shared_ptr<Animation> animation, uint32_t id, std::shared_ptr<Drawable> target, uint32_t spriteOffset = 0);

    bool step();
	void interrupt(int32_t label, bool setVisible = true);

private:
    void setSprite(int32_t id);
	void setOffset(float x, float y, float z) { target->setOffset({ x, y, z }); }
	void setScale(float x, float y) { target->setScale({ x, y }); }
	void setRotation(float x, float y, float z) { target->setRotation({ x, y, z }); }
	void setColor(uint8_t r, uint8_t g, uint8_t b) { target->setRGBColor({ r, g, b }); }
	void setAlpha(uint8_t alpha) { target->setAlpha(alpha); }
	void setVisibility(bool visible) { target->setVisible(visible); }
	void setBlendMode(uint8_t mode) { target->setBlendMode(mode); }
    void scrollTexture(float dx, float dy);
	void flipX(bool flip) { target->setFlipX(!target->isFlipX()); }
	void flipY(bool flip) { target->setFlipY(!target->isFlipY()); }
	void setAnchorTopLeft(bool anchor) { target->setCornerRelativePlacement(anchor); }
	void setOffsetSpeed(float x, float y, float z) { target->setOffsetSpeed(Vector(x, y, z)); }
	void setScaleSpeed(float x, float y) { target->setScaleSpeed(Point(x, y)); }
	void setRotationSpeed(float x, float y, float z) { target->setRotationSpeed(Vector(x, y, z)); }
	void setZWriteEnabled(bool enabled) { target->setZWriteEnabled(enabled); }
	void fadeTo(uint32_t alpha, uint32_t duration, uint8_t mode) { target->fade(duration, alpha, static_cast<InterpolationMode>(mode)); }
	void moveTo(float x, float y, float z, uint32_t duration, uint8_t mode) { target->moveTo(duration, { x, y, z }, static_cast<InterpolationMode>(mode)); }
	void rotateTo(float x, float y, float z, uint32_t duration, uint8_t mode) { target->rotateTo(duration, { x, y, z }, static_cast<InterpolationMode>(mode)); }
	void scaleTo(float x, float y, uint32_t duration, uint8_t mode) { target->scaleTo(duration, { x, y }, static_cast<InterpolationMode>(mode)); }

    std::shared_ptr<Animation> animation;
    std::shared_ptr<Drawable> target;

    bool running = true;
    bool waiting = false;

    ANMScript* script = nullptr;

    Timer frame;
    uint32_t instructionIndex = 0;
    uint32_t spriteOffset = 0;
};

