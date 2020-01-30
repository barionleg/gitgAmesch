#include <GigaMesh/logging/Logger.h>
#include <iostream>

Logger::Logger(bool isEnabled) : mIsEnabled(isEnabled)
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
