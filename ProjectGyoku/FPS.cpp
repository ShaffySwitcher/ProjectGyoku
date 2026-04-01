#include "FPS.h"
#include "Utils.h"
#include "Global.h"
#include <DxLib.h>

int FPS::targetFPS = 60;
float FPS::fps = static_cast<float>(targetFPS);
int FPS::startTime = 0;
int FPS::count = 0;
Text FPS::text;

void FPS::init() {
	text = Text(GetVector(WINDOW_WIDTH - 16.0f, WINDOW_HEIGHT - 16.0f), format("%.2ffps", fps), TextAlign::RIGHT);
	text.setCornerRelativePlacement(true);
}

void FPS::update() {
	if (count == 0) {
		startTime = GetNowCount();
	}
	if (count == targetFPS){
		const int now = GetNowCount();
		const int elapsedMS = now - startTime;

		if (elapsedMS > 0) {
			fps = (static_cast<float>(targetFPS) * 1000.0f) / static_cast<float>(elapsedMS);
		}
		
		text.setText(format("%.2ffps", fps));

		count = 0;
		startTime = now;
	}
	count++;
}

void FPS::setFPS(int fps)
{
	count = 0;
	targetFPS = fps;
}

void FPS::wait()
{
	int tookMilliseconds = GetNowCount() - startTime;
	int waitMilliseconds = count * 1000 / targetFPS - tookMilliseconds;
	if (waitMilliseconds > 0) {
		WaitTimer(waitMilliseconds);
	}
}

void FPS::render()
{
	text.render();
}
