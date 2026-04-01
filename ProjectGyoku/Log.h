#pragma once

class Log
{
public:
	static bool init();
	static void close();

	static void write(const char* format, ...); // file
	static void print(const char* format, ...); // console
	static void error(const char* format, ...); // message box + crash
	static void warn(const char* format, ...);  // message box
};