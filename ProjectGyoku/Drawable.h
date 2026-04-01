#pragma once
#include "Math.h"
#include "Interpolation.h"

enum class HorizontalAlign { LEFT, CENTER, RIGHT };
enum class VerticalAlign { TOP, CENTER, BOTTOM };

class Drawable {
public:
	virtual ~Drawable() = default;

	void update();

	void fade(unsigned int duration, uint8_t alpha, InterpolationMode mode = InterpolationMode::LINEAR);
	void tint(unsigned int duration, RGBColor color, InterpolationMode mode = InterpolationMode::LINEAR);
	void scaleTo(unsigned int duration, Point scale, InterpolationMode mode = InterpolationMode::LINEAR);
	void rotateTo(unsigned int duration, Vector rotation, InterpolationMode mode = InterpolationMode::LINEAR);
	void moveTo(unsigned int duration, Vector offset, InterpolationMode mode = InterpolationMode::LINEAR);

	void setVisible(bool visible) { this->visible = visible; }

	void setSizeOverride(Point size) { this->sizeOverride = size; }

	void setScale(Point scale) { this->scale = scale; }
	void setScaleSpeed(Point scaleSpeed) { this->scaleSpeed = scaleSpeed; }

	void setRotation(Vector rotation) { this->rotation = rotation; }
	void setRotationSpeed(Vector rotationSpeed) { this->rotationSpeed = rotationSpeed; }

	void setOffset(Vector offset) { this->offset = offset; }
	void setOffsetSpeed(Vector offsetSpeed) { this->offsetSpeed	= offsetSpeed; }

	void setColor(Color color) { this->color = color; }
	void setRGBColor(RGBColor rgb) { this->color.asRGB() = rgb; }
	void setAlpha(uint8_t alpha) { this->color.alpha = alpha; }
	void setBlendMode(int blendMode) { this->blendMode = blendMode; }

	void setFlipX(bool flip) { this->flipX = flip; }
	void setFlipY(bool flip) { this->flipY = flip; }

	void setCornerRelativePlacement(bool relative) { this->cornerRelativePlacement = relative; }

	void setZBufferEnabled(bool enabled) { this->zBufferEnabled = enabled; }
	void setZWriteEnabled(bool enabled) { this->zWriteEnabled = enabled; }

	bool isVisible() const { return this->visible; }

	Point getSizeOverride() const { return this->sizeOverride; }
	Point getScale() const { return this->scale; }
	Point getScaleSpeed() const { return this->scaleSpeed; }

	Vector getRotation() const { return this->rotation; }
	Vector getRotationSpeed() const { return this->rotationSpeed; }
	Vector getOffset() const { return this->offset; }

	Color getColor() const { return this->color; }
	RGBColor getRGBColor() const { return this->color.asRGB(); }
	uint8_t getAlpha() const { return this->color.alpha; }
	int getBlendMode() const { return this->blendMode; }

	bool isFlipX() const { return this->flipX; }
	bool isFlipY() const { return this->flipY; }
	bool isCornerRelativePlacement() const { return this->cornerRelativePlacement; }
	bool isZBufferEnabled() const { return this->zBufferEnabled; }
	bool isZWriteEnabled() const { return this->zWriteEnabled; }

	Interpolator<RGBColor>* getColorInterpolator() const { return this->colorInterpolator.get(); }
	Interpolator<uint8_t>* getAlphaInterpolator() const { return this->alphaInterpolator.get(); }
	Interpolator<Point>* getScaleInterpolator() const { return this->scaleInterpolator.get(); }
	Interpolator<Vector>* getRotationInterpolator() const { return this->rotationInterpolator.get(); }
	Interpolator<Vector>* getOffsetInterpolator() const { return this->offsetInterpolator.get(); }

	virtual void render(Vector position) = 0;
protected:
	bool visible = true;

	Point sizeOverride = GetPoint(-1.0f, -1.0f);

	Vector offset = GetVector(0.0f, 0.0f);
	Vector offsetSpeed = GetVector(0.0f, 0.0f);

	Point scale = GetPoint(1.0f, 1.0f);
	Point scaleSpeed = GetPoint(0.0f, 0.0f);

	Vector rotation = GetVector(0.0f, 0.0f, 0.0f);
	Vector rotationSpeed = GetVector(0.0f, 0.0f, 0.0f);

	Color color = Color::White();

	bool flipX = false;
	bool flipY = false;
	bool cornerRelativePlacement = false;

	int blendMode = DX_BLENDMODE_ALPHA;
	bool zBufferEnabled = false;
	bool zWriteEnabled = false;

	std::shared_ptr<Interpolator<RGBColor>> colorInterpolator = nullptr;
	std::shared_ptr<Interpolator<uint8_t>> alphaInterpolator = nullptr;
	std::shared_ptr<Interpolator<Point>> scaleInterpolator = nullptr;
	std::shared_ptr<Interpolator<Vector>> rotationInterpolator = nullptr;
	std::shared_ptr<Interpolator<Vector>> offsetInterpolator = nullptr;

	Timer frame;
};