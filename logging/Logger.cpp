#include "Logger.h"
#include <iostream>

Logger::Logger(bool isEnabled) : mOutputStream(nullptr), mIsEnabled(isEnabled)
{

}

Logger::~Logger()
{
	if(mOutputStream)
		mOutputStream->flush();
}

bool Logger::isEnabled() const
{
	return mIsEnabled;
}

void Logger::setIsEnabled(bool isEnabled)
{
	mIsEnabled = isEnabled;
}

void Logger::setOutputStream(std::ostream* outputStream)
{
	mOutputStream = outputStream;
}
