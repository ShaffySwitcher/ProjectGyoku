#pragma once

#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/Drawable.h"

class Background {
public:
	void setTexture(Texture* texture) { this->texture = texture; }
	
	void render();

private:
	Texture* texture = nullptr;
};

class Sprite : public Drawable
{
public:
	void render(Vector position);

	void setTexture(Texture* texture) { this->texture = texture; }
	void setTexCoords(Rect coords) { this->texCoords = coords; }
	void setTexOffset(Point offset) { this->texOffset = offset; }

	Rect getTexCoords() const { return this->texCoords; }
	Point getTexOffset() const { return this->texOffset; }

private:
	Rect texCoords = GetRectangle(0.0f, 0.0f, 0.0f, 0.0f);
	Point texOffset = Point(0.0f, 0.0f);

	Texture* texture = nullptr;
};

