#include "Engine/Graphics/Drawable.h"
#include "Engine/Supervisor.h"

void Drawable::update()
{
	if (this->rotationSpeed.x || this->rotationSpeed.y || this->rotationSpeed.z) {
		this->rotation += this->rotationSpeed * gGameManager.gameSpeed;
	}
	else if (this->rotationInterpolator) {
		this->rotationInterpolator->update(this->frame.getFrame());
		this->rotation = this->rotationInterpolator->getValue();
		if(this->rotationInterpolator->isFinished()) {
			this->rotationInterpolator.reset();
		}
	}

	if (this->scaleSpeed.x || this->scaleSpeed.y) {
		this->scale += this->scaleSpeed * gGameManager.gameSpeed;
	}
	else if(this->scaleInterpolator) {
		this->scaleInterpolator->update(this->frame.getFrame());
		this->scale = this->scaleInterpolator->getValue();
		if(this->scaleInterpolator->isFinished()) {
			this->scaleInterpolator.reset();
		}
	}

	if (this->offsetSpeed.x || this->offsetSpeed.y || this->offsetSpeed.z) {
		this->offset += this->offsetSpeed * gGameManager.gameSpeed;
	} else if (this->offsetInterpolator) {
		this->offsetInterpolator->update(this->frame.getFrame());
		this->offset = this->offsetInterpolator->getValue();
		if(this->offsetInterpolator->isFinished()) {
			this->offsetInterpolator.reset();
		}
	}

	if(this->colorInterpolator) {
		this->colorInterpolator->update(this->frame.getFrame());
		this->color.asRGB() = this->colorInterpolator->getValue();
		if(this->colorInterpolator->isFinished()) {
			this->colorInterpolator.reset();
		}
	}

	if(this->alphaInterpolator) {
		this->alphaInterpolator->update(this->frame.getFrame());
		this->color.alpha = this->alphaInterpolator->getValue();
		if(this->alphaInterpolator->isFinished()) {
			this->alphaInterpolator.reset();
		}
	}

	this->frame++;
}

void Drawable::fade(unsigned int duration, uint8_t alpha, InterpolationMode mode)
{
	this->alphaInterpolator = std::make_unique<Interpolator<uint8_t>>(
		this->color.alpha,
		this->frame.getFrame(),
		alpha,
		this->frame.getFrame() + duration,
		mode
	);
}

void Drawable::tint(unsigned int duration, RGBColor color, InterpolationMode mode)
{
	this->colorInterpolator = std::make_unique<Interpolator<RGBColor>>(
		this->color.asRGB(),
		this->frame.getFrame(),
		color,
		this->frame.getFrame() + duration,
		mode
	);
}

void Drawable::scaleTo(unsigned int duration, Point scale, InterpolationMode mode)
{
	this->scaleInterpolator = std::make_unique<Interpolator<Point>>(
		this->scale,
		this->frame.getFrame(),
		scale,
		this->frame.getFrame() + duration,
		mode
	);
}

void Drawable::rotateTo(unsigned int duration, Vector rotation, InterpolationMode mode)
{
	this->rotationInterpolator =  std::make_unique<Interpolator<Vector>>(
		this->rotation,
		this->frame.getFrame(),
		rotation,
		this->frame.getFrame() + duration,
		mode
	);
}

void Drawable::moveTo(unsigned int duration, Vector offset, InterpolationMode mode)
{
	this->offsetInterpolator = std::make_unique<Interpolator<Vector>>(
		this->offset,
		this->frame.getFrame(),
		offset,
		this->frame.getFrame() + duration,
		mode
	);
}
