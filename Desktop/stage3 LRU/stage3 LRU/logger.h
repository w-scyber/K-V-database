#pragma once
#include <stdio.h>
#define _CRT_SECURE_NO_WARNINGS
#define LOG_BUFFER_SIZE 1024
class logger
{
public:
	logger(const char* filepath);
	~logger(void);
	bool write(const char* format, ...);
private:
	static int preMark(char* buffer);
private:
	FILE* fp;
	char   m_buffer[LOG_BUFFER_SIZE];
};