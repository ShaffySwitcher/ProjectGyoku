#pragma once

#include <random>

class RNG {
public:
	RNG(unsigned int seed) : seed(seed), engine(seed) {}

	void setSeed(unsigned int seed);
	unsigned int getSeed() const;

	int getInt();			// returns [INT_MIN, INT_MAX]
	unsigned int getUInt(); // returns [0, UINT_MAX]
	float getFloat();		// returns [0.0, 1.0[

	int getIntRange(int min, int max);								// returns [min, max]
	unsigned int getUIntRange(unsigned int min, unsigned int max);
	float getFloatRange(float min, float max);						// returns [min, max[

private:
	unsigned int seed;
	std::mt19937 engine;
};