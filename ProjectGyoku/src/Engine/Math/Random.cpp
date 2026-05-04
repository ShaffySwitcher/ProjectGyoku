#include "Engine/Math/Random.h"
#include <climits>
#include <random>

void RNG::setSeed(unsigned int seed) {
	this->seed = seed;
	engine.seed(seed);
}

unsigned int RNG::getSeed() const
{
	return this->seed;
}

int RNG::getInt()
{
	return this->getIntRange(INT_MIN, INT_MAX);
}

unsigned int RNG::getUInt()
{
	return this->getUIntRange(0, UINT_MAX);
}

float RNG::getFloat()
{
	return this->getFloatRange(0.0f, 1.0f);
}

int RNG::getIntRange(int min, int max)
{
	std::uniform_int_distribution<int> distribution(min, max); return distribution(engine);
}

unsigned int RNG::getUIntRange(unsigned int min, unsigned int max)
{
	std::uniform_int_distribution<unsigned int> distribution(min, max); return distribution(engine);
}

float RNG::getFloatRange(float min, float max)
{
	std::uniform_real_distribution<float> distribution(min, max); return distribution(engine);
}

