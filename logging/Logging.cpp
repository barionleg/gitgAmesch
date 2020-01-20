#include "Logging.h"
#include "Logger.h"

#include <iostream>

namespace LOG {
	Logger logFatal (true );
	Logger logError (true );
	Logger logWarn  (false);
	Logger logInfo  (false);
	Logger logDebug (false);

	void initLogging()
	{
		logFatal.setOutputStream (&std::cerr);
		logError.setOutputStream (&std::cerr);
		logWarn. setOutputStream (&std::cerr);
		logInfo. setOutputStream (&std::cout);
		logDebug.setOutputStream (&std::cerr);
	}

	Logger* getLogger(LogLevel level)
	{
		Logger* logger = nullptr;
		switch (level) {
			case LogLevel::eFatal:
				logger = &logFatal;
				break;
			case LogLevel::eError:
				logger = &logError;
				break;
			case LogLevel::eWarn:
				logger = &logWarn;
				break;
			case LogLevel::eInfo:
				logger = &logInfo;
				break;
			case LogLevel::eDebug:
				logger = &logDebug;
				break;
		}

		return logger;
	}

	void setLoggerEnabled(bool enable, LogLevel level)
	{
		if(auto logger = getLogger(level); logger != nullptr)
			logger->setIsEnabled(enable);
	}

	void setLogStream(std::ostream* streamObject, LogLevel level)
	{
		if(auto logger = getLogger(level); logger != nullptr)
			logger->setOutputStream(streamObject);
	}

	void setLogLevel(LogLevel level)
	{
		auto l = static_cast<unsigned char>(level);
		for(auto i = static_cast<unsigned char>(LogLevel::eFatal); i <= static_cast<unsigned char>(LogLevel::eDebug); ++i)
		{
			setLoggerEnabled(l >= i, static_cast<LogLevel>(i));
		}
	}

	Logger& fatal()
	{
		return logFatal << "[LOG FATAL  ] ";
	}

	Logger& error()
	{
		return logError << "[LOG ERROR  ] ";
	}

	Logger& warn()
	{
		return logWarn << "[LOG WARNING] ";
	}

	Logger& info()
	{
		return logInfo << "[LOG INFO   ] ";
	}

	Logger& debug()
	{
		return logDebug << "[LOG DEBUG  ] ";
	}
}
