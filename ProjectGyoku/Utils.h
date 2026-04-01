#pragma once

#include <string>
#include <cstdarg>

#define MAX_FMT_STR_LEN 512

std::string format(const char* format, ...);
std::string fmt_str(const char* format, va_list args);

template<typename T>
T clamp(T value, T min, T max) {
	if (value < min) { return min; }
	if (value > max) { return max; }
	return value;
}