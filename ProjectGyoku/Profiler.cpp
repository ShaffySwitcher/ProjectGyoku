#include "Profiler.h"

#include <DxLib.h>

#include <algorithm>
#include <map>

#include "Utils.h"

namespace {
	std::map<std::string, double> frameSamples;
}

Profiler::Clock::time_point Profiler::frameStart = Profiler::Clock::now();
bool Profiler::enabled = false;
bool Profiler::overlayEnabled = true;
double Profiler::lastFrameMilliseconds = 0.0;
std::vector<Profiler::Entry> Profiler::lastEntries{};

void Profiler::beginFrame()
{
	frameStart = Clock::now();

	if (!enabled) {
		frameSamples.clear();
	}
}

void Profiler::endFrame()
{
	const Clock::time_point end = Clock::now();
	lastFrameMilliseconds = std::chrono::duration<double, std::milli>(end - frameStart).count();

	if (!enabled) {
		lastEntries.clear();
		frameSamples.clear();
		return;
	}

	lastEntries.clear();	
	lastEntries.reserve(frameSamples.size());
	for (const auto& sample : frameSamples) {
		lastEntries.push_back({ sample.first, sample.second });
	}

	std::sort(lastEntries.begin(), lastEntries.end(), [](const Entry& a, const Entry& b) {
		return a.milliseconds > b.milliseconds;
	});

	frameSamples.clear();
}

void Profiler::addSample(const char* name, double milliseconds)
{
	if (!enabled || !name || name[0] == '\0') {
		return;
	}

	frameSamples[name] += milliseconds;
}

bool Profiler::isEnabled()
{
	return enabled;
}

void Profiler::setEnabled(bool enabled)
{
	Profiler::enabled = enabled;
	if (!enabled) {
		lastEntries.clear();
		frameSamples.clear();
	}
}

void Profiler::toggleEnabled()
{
	setEnabled(!enabled);
}

bool Profiler::isOverlayEnabled()
{
	return overlayEnabled;
}

void Profiler::setOverlayEnabled(bool enabled)
{
	overlayEnabled = enabled;
}

void Profiler::toggleOverlayEnabled()
{
	overlayEnabled = !overlayEnabled;
}

double Profiler::getLastFrameMilliseconds()
{
	return lastFrameMilliseconds;
}

double Profiler::getLastSampleMilliseconds(const char* name)
{
	if (!name) {
		return 0.0;
	}

	for (const auto& entry : lastEntries) {
		if (entry.name == name) {
			return entry.milliseconds;
		}
	}

	return 0.0;
}

std::vector<Profiler::Entry> Profiler::getTopEntries(size_t maxEntries)
{
	if (maxEntries >= lastEntries.size()) {
		return lastEntries;
	}

	return std::vector<Entry>(lastEntries.begin(), lastEntries.begin() + maxEntries);
}

void Profiler::renderOverlay(int x, int y, size_t maxEntries)
{
	if (!enabled || !overlayEnabled) {
		return;
	}

	const std::vector<Entry> entries = getTopEntries(maxEntries);
	const int lineHeight = 18;
	const int boxWidth = 280;
	const int lineCount = static_cast<int>(entries.size()) + 1;
	const int boxHeight = (lineCount * lineHeight) + 14;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	DrawBox(x, y, x + boxWidth, y + boxHeight, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawBox(x, y, x + boxWidth, y + boxHeight, GetColor(170, 170, 170), FALSE);

	DrawString(x + 8, y + 6, format("Frame: %.2f ms", lastFrameMilliseconds).c_str(), GetColor(255, 255, 255));

	for (size_t i = 0; i < entries.size(); i++) {
		const int lineY = y + 6 + (lineHeight * static_cast<int>(i + 1));
		DrawString(
			x + 8,
			lineY,
			format("%s: %.2f ms", entries[i].name.c_str(), entries[i].milliseconds).c_str(),
			GetColor(210, 210, 210)
		);
	}
}

ScopedProfilerSample::ScopedProfilerSample(const char* sampleName)
	: name(sampleName), start(std::chrono::steady_clock::now())
{
}

ScopedProfilerSample::~ScopedProfilerSample()
{
	if (!Profiler::isEnabled()) {
		return;
	}

	const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	const double elapsed = std::chrono::duration<double, std::milli>(end - start).count();
	Profiler::addSample(name, elapsed);
}
