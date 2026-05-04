#pragma once

#include "Engine/Graphics/Drawable.h"
#include "Engine/Graphics/Animation.h"

#define TEXT_SP_SYMBOL -128
#define TEXT_END_SYMBOL -127

using TextAlign = HorizontalAlign;

struct TextGradientColor {
	Color top;
	Color bottom;

	static TextGradientColor default() {
		return TextGradientColor{ Color::White(), Color::getColor(102, 51, 153)};
	}
};

struct TextInfo {
	Point size;
	int lineCount;
};

class TextBase : public Drawable {
public:
	TextBase(Vector position = GetVector(0,0), std::string text = "", TextAlign align = TextAlign::LEFT);

	void update();

	void setText(const std::string& text) { this->text = text; }
	void setPosition(Vector position) { this->position = position; }
	void setAlign(TextAlign align) { this->align = align; }

	std::string getText() const { return this->text; }
	Vector getPosition() const { return this->position; }

protected:
	Vector position;
	std::string text;

	TextAlign align;
};

class NativeText : public TextBase
{
public:
	NativeText(Vector position = GetVector(0,0,0), std::string text = "", TextAlign align = TextAlign::LEFT, bool shadow = true, TextGradientColor gradient = TextGradientColor::default());

	static bool init();
	static void restore();

	void render(Vector position = GetVector(0.0f, 0.0f, 0.0f));

	void setGradient(TextGradientColor gradient) { this->gradient = gradient; }

	TextInfo getTextInfo() const {
		TextInfo textInfo;
		int width = 0, height = 0, lines = 0;
		GetDrawStringSizeToHandle(&width, &height, &lines, text.c_str(), text.size(), defaultFontHandle);
		textInfo.size.x = static_cast<float>(width);
		textInfo.size.y = static_cast<float>(height);
		textInfo.lineCount = lines;
		return textInfo;
	}

	static int defaultFontHandle;
private:
	int fontHandle;
	bool shadow;
	TextGradientColor gradient;
};

class Text : public TextBase {
public:
	Text(Vector position = GetVector(0, 0), std::string text = "", TextAlign align = TextAlign::LEFT, int xSpacing = 14, unsigned int shift = 32);

	static bool init();

	void render(Vector position = GetVector(0.0f, 0.0f, 0.0f));

	static std::shared_ptr<Animation> defaultFontAnimation;

private:
	std::shared_ptr<Animation> fontAnimation;
	int xSpacing;
	unsigned int shift;
};