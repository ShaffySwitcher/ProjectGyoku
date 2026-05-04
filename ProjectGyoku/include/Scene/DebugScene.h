#pragma once

#include "Engine/State.h"
#include "Engine/Graphics/Text.h"
#include "Engine/Graphics/Sprite.h"
#include "Engine/Graphics/Animation.h"

class DebugScene : public State
{
public:
	virtual void init() override;
	virtual void update() override;
	virtual void render() override;
	virtual void restore() override;
private:
	std::shared_ptr<ANMRunner> anmRunner;
	std::shared_ptr<Sprite> sprite;
	Text titleText;
	NativeText text;

	int32_t currentScript;

	void updateText();
};

