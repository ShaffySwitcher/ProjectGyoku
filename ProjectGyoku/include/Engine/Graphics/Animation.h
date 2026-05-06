#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <map>
#include <vector>
#include <memory>
#include <string>
#include "Engine/Global.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/Drawable.h"
#include "Engine/Graphics/AnimationInstructions.h"
#include "Engine/Format/ANM.h"

class ANMManager
{
public:
    static std::shared_ptr<ANM> load(const std::string& name, const std::string& path = "");
	static void unload(const std::string& name);
	static void unloadAll();

	static bool reloadScripts(const std::string& name);
	static int reloadScripts();
    static int reloadScriptsAndRestartRunners();

	static int reloadTextures();

    static void restore();

private:
	static std::map<std::string, std::shared_ptr<ANM>> animations;
};

class ANMRunner
{
public:
	ANMRunner(std::shared_ptr<ANM> animation, uint32_t id, std::shared_ptr<Drawable> target, uint32_t spriteOffset = 0);
    ~ANMRunner();

    bool step();
	void interrupt(int32_t label, bool setVisible = true);
    void restart();

    static void restartAll();

private:
    bool bindScript(bool logOnFailure = false);
    void refreshScriptBinding();

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

    std::shared_ptr<ANM> animation;
    std::shared_ptr<Drawable> target;

    bool running = true;
    bool waiting = false;

    int32_t scriptId = -1;
    uint32_t boundRevision = 0;
    bool missingScriptLogged = false;

    ANMScript* script = nullptr;

    Timer frame;
    uint32_t instructionIndex = 0;
    uint32_t spriteOffset = 0;

	static std::vector<ANMRunner*> activeRunners;
};

