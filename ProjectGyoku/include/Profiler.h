#pragma once

#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

class Profiler
{
public:
	struct Entry {
		std::string name;
		double milliseconds = 0.0;
	};

	static void beginFrame();
	static void endFrame();

	static void addSample(const char* name, double milliseconds);

	static bool isEnabled();
	static void setEnabled(bool enabled);
	static void toggleEnabled();

	static bool isOverlayEnabled();
	static void setOverlayEnabled(bool enabled);
	static void toggleOverlayEnabled();

	static double getLastFrameMilliseconds();
	static double getLastSampleMilliseconds(const char* name);
	static std::vector<Entry> getTopEntries(size_t maxEntries);

	static void renderOverlay(int x = 12, int y = 12, size_t maxEntries = 6);

private:
	using Clock = std::chrono::steady_clock;

	static Clock::time_point frameStart;
	static bool enabled;
	static bool overlayEnabled;
	static double lastFrameMilliseconds;
	static std::vector<Entry> lastEntries;
};

class ScopedProfilerSample
{
public:
	explicit ScopedProfilerSample(const char* sampleName);
	~ScopedProfilerSample();

private:
	const char* name;
	std::chrono::steady_clock::time_point start;
};

#define PROFILE_SCOPE(nameLiteral) ScopedProfilerSample scopedProfilerSample_##__LINE__(nameLiteral)
