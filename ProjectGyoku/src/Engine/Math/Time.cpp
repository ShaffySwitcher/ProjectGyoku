#include "Engine/Math/Time.h"
#include <Windows.h>

DateTime DateTime::now()
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	DateTime dt;
	dt.year = static_cast<uint16_t>(st.wYear);
	dt.month = static_cast<uint8_t>(st.wMonth);
	dt.day = static_cast<uint8_t>(st.wDay);
	dt.hours = static_cast<uint8_t>(st.wHour);
	dt.minutes = static_cast<uint8_t>(st.wMinute);
	dt.seconds = static_cast<uint8_t>(st.wSecond);

	return dt;
}

DateTime &DateTime::operator+=(const DateTime &other)
{
	this->seconds += other.seconds;
	this->minutes += other.minutes + this->seconds / 60;
	this->seconds %= 60;

	this->hours += other.hours + this->minutes / 60;
	this->minutes %= 60;

	this->day += other.day + this->hours / 24;
	this->hours %= 24;

	// this does not account for varying month lengths or leap years, but for gameplay tracking, it should be fine...
	this->month += other.month + (this->day - 1) / 30;
	this->day = ((this->day - 1) % 30) + 1;

	this->year += other.year + (this->month - 1) / 12;
	this->month = ((this->month - 1) % 12) + 1;

	return *this;
}

DateTime &DateTime::operator+=(const int seconds)
{
    return *this += DateTime{ 0, 0, 0, 0, 0, static_cast<uint8_t>(seconds) };
}

