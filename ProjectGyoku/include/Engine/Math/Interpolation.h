#pragma once

#include "Engine/Math/GyokuMath.h"

enum class InterpolationMode {
	LINEAR,
	
	// Sine
	EASE_IN_SINE,
	EASE_OUT_SINE,
	EASE_IN_OUT_SINE,

	// Quad
	EASE_IN_QUAD,
	EASE_OUT_QUAD,
	EASE_IN_OUT_QUAD,

	// Cubic
	EASE_IN_CUBIC,
	EASE_OUT_CUBIC,
	EASE_IN_OUT_CUBIC,

	// Quart
	EASE_IN_QUART,
	EASE_OUT_QUART,
	EASE_IN_OUT_QUART,

	// Quint
	EASE_IN_QUINT,
	EASE_OUT_QUINT,
	EASE_IN_OUT_QUINT,

	// Expo
	EASE_IN_EXPO,
	EASE_OUT_EXPO,
	EASE_IN_OUT_EXPO,

	// Circ
	EASE_IN_CIRC,
	EASE_OUT_CIRC,
	EASE_IN_OUT_CIRC,

	// Back
	EASE_IN_BACK,
	EASE_OUT_BACK,
	EASE_IN_OUT_BACK,

	// Elastic
	EASE_IN_ELASTIC,
	EASE_OUT_ELASTIC,
	EASE_IN_OUT_ELASTIC,

	// Bounce
	EASE_IN_BOUNCE,
	EASE_OUT_BOUNCE,
	EASE_IN_OUT_BOUNCE

};

template <class T>
class Interpolator {
public:
	Interpolator(T value, unsigned int startFrame, T endValue, unsigned int endFrame, InterpolationMode mode = InterpolationMode::LINEAR);

	T getValue() { return value; };
	bool isFinished() { return finished; }
	unsigned int getEndFrame() { return this->endFrame; }

	void setInterpolationStartValues(T startValue);
	void setInterpolationStartFrame(unsigned int startFrame);
	void setInterpolationStart(unsigned int startFrame, T value);

	void setInterpolationEndValues(T endValue);
	void setInterpolationEndFrame(unsigned int endFrame);
	void setInterpolationEnd(unsigned int endFrame, T endValue);

	T lerp(const T& start, const T& end, float coefficient);

	static float easeOutBounce(float x);

	void update(unsigned int frame);
private:
	unsigned int startFrame, endFrame, frame;
	T value, startValue, endValue;

	InterpolationMode mode;

	bool finished = false;
};

template<class T>
inline Interpolator<T>::Interpolator(T value, unsigned int startFrame, T endValue, unsigned int endFrame, InterpolationMode mode)
{
	this->value = value;
	this->startValue = value;
	this->endValue = endValue;

	this->mode = mode;

	this->startFrame = startFrame;
	this->endFrame = endFrame;
	this->frame = 0;
	this->finished = false;
}

template<class T>
inline void Interpolator<T>::setInterpolationStart(unsigned int startFrame, T value)
{
	this->startValue = value;
	this->startFrame = startFrame;
	this->finished = false;
}


template<class T>
inline void Interpolator<T>::setInterpolationStartValues(T startValue)
{
	this->startValue = startValue;
	this->finished = false;
}


template<class T>
inline void Interpolator<T>::setInterpolationStartFrame(unsigned int startFrame)
{
	this->startFrame = startFrame;
	this->finished = false;
}


template<class T>
inline void Interpolator<T>::setInterpolationEndValues(T endValue)
{
	this->endValue = endValue;
	this->finished = false;
}


template<class T>
inline void Interpolator<T>::setInterpolationEndFrame(unsigned int endFrame)
{
	this->endFrame = endFrame;
	this->finished = false;
}


template<class T>
inline void Interpolator<T>::setInterpolationEnd(unsigned int endFrame, T endValue)
{
	this->endValue = endValue;
	this->endFrame = endFrame;
	this->finished = false;
}

template<class T>
inline T Interpolator<T>::lerp(const T& start, const T& end, float coefficient)
{
	return start + (end - start) * coefficient;
}

template<class T>
inline float Interpolator<T>::easeOutBounce(float x)
{
	const float n1 = 7.5625f;
	const float d1 = 2.75f;

	if (x < 1.0f / d1) {
		return n1 * x * x;
	}
	else if (x < 2.0f / d1) {
		x -= 1.5f / d1;
		return n1 * x * x + 0.75f;
	}
	else if (x < 2.5f / d1) {
		x -= 2.25f / d1;
		return n1 * x * x + 0.9375f;
	}
	else {
		x -= 2.625f / d1;
		return n1 * x * x + 0.984375f;
	}
}


template<class T>
inline void Interpolator<T>::update(unsigned int frame)
{
	if (finished) {
		return;
	}

	float coefficient = 0.0f;

	this->frame = frame;

	if (this->frame >= endFrame) {
		this->value = endValue;
		this->finished = true;
	}
	else {
		coefficient = static_cast<float>(frame - startFrame) / static_cast<float>(endFrame - startFrame);

		switch (mode) {
			case InterpolationMode::LINEAR:
				break;

			case InterpolationMode::EASE_IN_SINE:
				coefficient = 1.0f - cosf((coefficient * Angle::PI()) / 2.0f);
				break;
			case InterpolationMode::EASE_OUT_SINE:
				coefficient = sinf((coefficient * Angle::PI()) / 2.0f);
				break;
			case InterpolationMode::EASE_IN_OUT_SINE:
				coefficient = -(cosf(Angle::PI() * coefficient) - 1.0f) / 2.0f;
				break;

			case InterpolationMode::EASE_IN_QUAD:
				coefficient = powf(coefficient, 2.0f);
				break;
			case InterpolationMode::EASE_OUT_QUAD:
				coefficient = 1.0f - powf(1.0f - coefficient, 2.0f);
				break;
			case InterpolationMode::EASE_IN_OUT_QUAD:
				coefficient = coefficient < 0.5f ? 2.0f * powf(coefficient, 2.0f) : 1.0f - powf(-2.0f * coefficient + 2.0f, 2.0f) / 2.0f;
				break;

			case InterpolationMode::EASE_IN_CUBIC:
				coefficient = powf(coefficient, 3.0f);
				break;
			case InterpolationMode::EASE_OUT_CUBIC:
				coefficient = 1.0f - powf(1.0f - coefficient, 3.0f);
				break;
			case InterpolationMode::EASE_IN_OUT_CUBIC:
				coefficient = coefficient < 0.5f ? 4.0f * powf(coefficient, 3.0f) : 1.0f - powf(-2.0f * coefficient + 2.0f, 3.0f) / 2.0f;
				break;

			case InterpolationMode::EASE_IN_QUART:
				coefficient = powf(coefficient, 4.0f);
				break;
			case InterpolationMode::EASE_OUT_QUART:
				coefficient = 1.0f - powf(1.0f - coefficient, 4.0f);
				break;
			case InterpolationMode::EASE_IN_OUT_QUART:
				coefficient = coefficient < 0.5f ? 8.0f * powf(coefficient, 4.0f) : 1.0f - powf(-2.0f * coefficient + 2.0f, 4.0f) / 2.0f;
				break;

			case InterpolationMode::EASE_IN_QUINT:
				coefficient = powf(coefficient, 5.0f);
				break;
			case InterpolationMode::EASE_OUT_QUINT:
				coefficient = 1.0f - powf(1.0f - coefficient, 5.0f);
				break;
			case InterpolationMode::EASE_IN_OUT_QUINT:
				coefficient = coefficient < 0.5f ? 16.0f * powf(coefficient, 5.0f) : 1.0f - powf(-2.0f * coefficient + 2.0f, 5.0f) / 2.0f;
				break;

			case InterpolationMode::EASE_IN_EXPO:
				coefficient = coefficient == 0.0f ? 0.0f : powf(2.0f, 10.0f * coefficient - 10.0f);
				break;
			case InterpolationMode::EASE_OUT_EXPO:
				coefficient = coefficient == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * coefficient);
				break;
			case InterpolationMode::EASE_IN_OUT_EXPO:
				coefficient = coefficient == 0.0f ? 0.0f : coefficient == 1.0f ? 1.0f : coefficient < 0.5f ? powf(2.0f, 20.0f * coefficient - 10.0f) / 2.0f : (2.0f - powf(2.0f, -20.0f * coefficient + 10.0f)) / 2.0f;
				break;

			case InterpolationMode::EASE_IN_CIRC:
				coefficient = 1.0f - sqrtf(1.0f - powf(coefficient, 2.0f));
				break;
			case InterpolationMode::EASE_OUT_CIRC:
				coefficient = sqrtf(1.0f - powf(coefficient - 1.0f, 2.0f));
				break;
			case InterpolationMode::EASE_IN_OUT_CIRC:
				coefficient = coefficient < 0.5f ? (1.0f - sqrtf(1.0f - powf(2.0f * coefficient, 2.0f))) / 2.0f : (sqrtf(1.0f - powf(-2.0f * coefficient + 2.0f, 2.0f)) + 1.0f) / 2.0f;
				break;

			case InterpolationMode::EASE_IN_BACK:
				coefficient = 2.70158f * coefficient * coefficient * coefficient - 1.70158f * coefficient * coefficient;
				break;
			case InterpolationMode::EASE_OUT_BACK:
				coefficient = 1.0f + 2.70158f * powf(coefficient - 1.0f, 3.0f) + 1.70158f * powf(coefficient - 1.0f, 2.0f);
				break;
			case InterpolationMode::EASE_IN_OUT_BACK:
				coefficient = coefficient < 0.5f ? (powf(2.0f * coefficient, 2.0f) * ((2.70158f * 1.525f + 1.0f) * 2.0f * coefficient - 2.70158f * 1.525f)) / 2.0f : (powf(2.0f * coefficient - 2.0f, 2.0f) * ((2.70158f * 1.525f + 1.0f) * (coefficient * 2.0f - 2.0f) + 2.70158f * 1.525f) + 2.0f) / 2.0f;
				break;

			case InterpolationMode::EASE_IN_ELASTIC:
				coefficient = coefficient == 0.0f ? 0.0f : coefficient == 1.0f ? 1.0f : -powf(2.0f, 10.0f * coefficient - 10.0f) * sinf((coefficient * 10.0f - 10.75f) * ((2.0f * Angle::PI()) / 3.0f));
				break;
			case InterpolationMode::EASE_OUT_ELASTIC:
				coefficient = coefficient == 0.0f ? 0.0f : coefficient == 1.0f ? 1.0f : powf(2.0f, -10.0f * coefficient) * sinf((coefficient * 10.0f - 0.75f) * ((2.0f * Angle::PI()) / 3.0f)) + 1.0f;
				break;
			case InterpolationMode::EASE_IN_OUT_ELASTIC:
				coefficient = coefficient == 0.0f ? 0.0f : coefficient == 1.0f ? 1.0f : coefficient < 0.5f ? -(powf(2.0f, 20.0f * coefficient - 10.0f) * sinf((20.0f * coefficient - 11.125f) * ((2.0f * Angle::PI()) / 4.5f))) / 2.0f : (powf(2.0f, -20.0f * coefficient + 10.0f) * sinf((20.0f * coefficient - 11.125f) * ((2.0f * Angle::PI()) / 4.5f)) + 2.0f) / 2.0f;
				break;

			case InterpolationMode::EASE_IN_BOUNCE:
				coefficient = 1.0f - easeOutBounce(1.0f - coefficient);
				break;
			case InterpolationMode::EASE_OUT_BOUNCE:
				coefficient = easeOutBounce(coefficient);
				break;
			case InterpolationMode::EASE_IN_OUT_BOUNCE:
				coefficient = coefficient < 0.5f ? (1.0f - easeOutBounce(1.0f - 2.0f * coefficient)) / 2.0f : (1.0f + easeOutBounce(2.0f * coefficient - 1.0f)) / 2.0f;
				break;


		}

		this->value = static_cast<T>(startValue + coefficient * (endValue - startValue));
	}
}

