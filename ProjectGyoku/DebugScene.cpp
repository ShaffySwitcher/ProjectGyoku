#include "DebugScene.h"
#include "Log.h"
#include "Text.h"
#include "Animation.h"
#include "Input.h"
#include "Utils.h"

void DebugScene::init()
{
	titleText.setText(u8"[Debug Scene]");
	titleText.setCornerRelativePlacement(false);
	titleText.setAlign(TextAlign::CENTER);
	titleText.setPosition(GetVector(320, 20, 0));

	currentScript = ANMManager::load("dummy")->scripts.begin()->first;

    updateText();
	text.setAlign(TextAlign::CENTER);
	text.setPosition(GetVector(320, 60, 0));

	sprite = std::make_shared<Sprite>();
	anmRunner = std::make_shared<ANMRunner>(ANMManager::load("dummy"), currentScript, sprite);
}

void DebugScene::update()
{
	int oldScript = currentScript;
	auto& scripts = ANMManager::load("dummy")->scripts;

    if (Input::gameInputPressed[static_cast<uint8_t>(GameInput::LEFT)]) {
        auto it = scripts.find(currentScript);

        if (it != scripts.end()) {
            if (it == scripts.begin()) {
                it = scripts.end();
            }
            --it;
            currentScript = it->first;
        }
        else if (!scripts.empty()) {
            currentScript = scripts.rbegin()->first;
        }
    }

    if (Input::gameInputPressed[static_cast<uint8_t>(GameInput::RIGHT)]) {
        auto it = scripts.find(currentScript);

        if (it != scripts.end()) {
            ++it;

            if (it == scripts.end()) {
                it = scripts.begin();
            }

            currentScript = it->first;
        }
        else if (!scripts.empty()) {
            currentScript = scripts.begin()->first;
        }
    }

	if (currentScript != oldScript) {
		sprite = std::make_shared<Sprite>();
		anmRunner = std::make_shared<ANMRunner>(ANMManager::load("dummy"), currentScript, sprite);
        updateText();
	}

	titleText.update();
	text.update();

	anmRunner->step();
}

void DebugScene::render()
{
	ClearDrawScreen();

	titleText.render();

	sprite->render(GetVector(320, 240));

	text.render();
}

void DebugScene::restore()
{

}

void DebugScene::updateText()
{
    text.setText(format("Current Script: %i\n\n[Left] or [Right] to change script.", currentScript));
}
