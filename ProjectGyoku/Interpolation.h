#pragma once

#include "Math.h"

enum class InterpolationMode {
	LINEAR,
	EASE_OUT,
	EASE_IN,
	EASE_IN_OUT,
	EASE_IN_BACK,
	EASE_OUT_BACK,
	EASE_IN_OUT_BACK,
	EASE_IN_ELASTIC,
	EASE_OUT_ELASTIC,
	EASE_IN_OUT_ELASTIC,
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
		case InterpolationMode::EASE_IN:
			coefficient = std::powf(coefficient, 2);
			break;
		case InterpolationMode::EASE_OUT:
			coefficient = 1 - std::powf((1 - coefficient), 2);
			break;
		case InterpolationMode::EASE_IN_OUT:
			coefficient = (coefficient < 0.5f) ? (2.0f * coefficient * coefficient) : (1.0f - std::powf(-2.0f * coefficient + 2.0f, 2.0f) / 2.0f);
			break;
		case InterpolationMode::EASE_IN_BACK:
			coefficient = (1.70158f + 1) * coefficient * coefficient * coefficient - 1.70158f * coefficient * coefficient;
			break;
		case InterpolationMode::EASE_OUT_BACK:
			coefficient = 1 + (1.70158f + 1) * std::powf(coefficient - 1, 3) + 1.70158f * std::powf(coefficient - 1, 2);
			break;
		case InterpolationMode::EASE_IN_OUT_BACK:
			coefficient = coefficient < 0.5f
				? (std::powf(2.0f * coefficient, 2.0f) * (((1.70158f * 1.525f) + 1.0f) * 2.0f * coefficient - (1.70158f * 1.525f))) / 2.0f
				: (std::powf(2.0f * coefficient - 2.0f, 2.0f) * (((1.70158f * 1.525f) + 1.0f) * (coefficient * 2.0f - 2.0f) + (1.70158f * 1.525f)) + 2.0f) / 2.0f;
			break;
		case InterpolationMode::EASE_IN_ELASTIC:
			coefficient = coefficient == 0.0f
				? 0.0f
				: coefficient == 1.0f
				? 1.0f
				: -std::powf(2, 10.0f * coefficient - 10.0f) * std::sinf((coefficient * 10.0f - 10.75f) * (2.0f * static_cast<float>(Angle::PI()) / 3.0f));
			break;
		case InterpolationMode::EASE_OUT_ELASTIC:
			coefficient = coefficient == 0.0f
				? 0.0f
				: coefficient == 1.0f
				? 1.0f
				: std::powf(2, -10.0f * coefficient) * std::sinf((coefficient * 10.0f - 0.75f) * (2.0f * static_cast<float>(Angle::PI()) / 3.0f)) + 1;
			break;
		case InterpolationMode::EASE_IN_OUT_ELASTIC:
			coefficient = static_cast<float>(coefficient == 0.0f)
				? 0.0f
				: coefficient == 1.0f
				? 1.0f
				: coefficient < 0.5f
				? -(std::powf(2.0f, 20.0f * coefficient - 10.0f) * std::sinf((20.0f * coefficient - 11.125f) * ((2.0f * static_cast<float>(Angle::PI())) / 4.5f))) / 2.0f
				: (std::powf(2.0f, -20.0f * coefficient + 10.0f) * std::sinf((20.0f * coefficient - 11.125f) * ((2.0f * static_cast<float>(Angle::PI())) / 4.5f))) / 2.0f + 1;
			break;
		}

		this->value = static_cast<T>(startValue + coefficient * (endValue - startValue));
	}
}

