#include "Engine/Utils.h"
#include "Engine/Log.h"
#include <string>
#include <cstdarg>
#include <cstdio>
#include <sstream>
#include <string.h>

std::string format(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	std::string message = fmt_str(format, args);
	va_end(args);
	return message;
}

std::string fmt_str(const char* format, va_list args)
{
	std::ostringstream oss;
	char buffer[MAX_FMT_STR_LEN]{};

	if (strlen(format) >= MAX_FMT_STR_LEN) {
		Log::write("fmt_str: format string too long! (expected <%i, got %i)", MAX_FMT_STR_LEN, strlen(format));
	}
	else {
		vsnprintf_s(buffer, sizeof(buffer), format, args);
		oss << buffer;
	}

	return oss.str();
}
