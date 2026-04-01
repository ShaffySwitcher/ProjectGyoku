#include "Log.h"
#include <Windows.h>
#include <fstream>
#include "Utils.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ostream>

static std::ofstream logFile;

bool Log::init()
{
	logFile.open("log.txt");
	if (!logFile.is_open()) {
		return false;
	}

	// Debug Console
#ifdef DEBUG
	FreeConsole();
	AllocConsole();

	// attach the new console to this application's process
	AttachConsole(GetCurrentProcessId());

	SetConsoleTitle("Project Gyoku - Debug Console");

	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
#endif

	Log::write("------------------------ Project Gyoku ------------------------");

	return true;
}

void Log::close() {
	Log::write("--------------------- Thanks for playing!  --------------------");

	if (logFile.is_open()) {
		logFile.close();
	}
}

void Log::write(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	std::string message = fmt_str(format, args);
	va_end(args);

	if (logFile.is_open()) {
		logFile << message << std::endl;
	}

	Log::print(("<Log> " + message).c_str());
}

void Log::print(const char* format, ...)
{
#ifdef DEBUG
	va_list args;
	va_start(args, format);
	printf("%s\n", fmt_str(format, args).c_str());
	va_end(args);
#endif
}

void Log::error(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	std::string message = fmt_str(format, args);
	va_end(args);

	MessageBoxA(NULL, message.c_str(), "Project Gyoku Error", MB_ICONERROR);
	Log::write(("<Error> " + message).c_str());

	Log::close();
	exit(1);
}

void Log::warn(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	std::string message = fmt_str(format, args);
	va_end(args);

	MessageBoxA(NULL, message.c_str(), "Project Gyoku Information", MB_ICONINFORMATION);
	Log::write(("<Warning> " + message).c_str());
}

