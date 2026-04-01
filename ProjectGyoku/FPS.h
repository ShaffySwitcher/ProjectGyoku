#pragma once
#include "Text.h"

class FPS
{
public:
	static float fps;

	static void init();
	static void update();
	static void setFPS(int fps);
	static void wait();
	static void render();

	static int getFPS() { return static_cast<int>(fps); }

private:
	static int startTime;
	static int count;
	static int targetFPS;
	static Text text;
};

