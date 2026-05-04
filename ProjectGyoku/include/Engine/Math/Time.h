#pragma once

#include "Engine/Math/GyokuMath.h"

struct DateTime {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;

	static DateTime now();

	DateTime& operator+=(const DateTime& other);
	DateTime& operator+=(const int seconds);
};