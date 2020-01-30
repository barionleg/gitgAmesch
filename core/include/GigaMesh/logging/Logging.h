#ifndef LOGGING_H
#define LOGGING_H

#include "Logger.h"

namespace LOG{
	enum class LogLevel : unsigned char {
		eFatal,
		eError,
		eWarn,
		eInfo,
		eDebug
	};

	//Convenience functions to write logs in a stream-like fashion to std::cerr or std::cout
	//Call initLogging() from any place first before using Those!
	//Example usage: Log::fatal() << "myMessage\n";
	Logger& fatal();	//! log fatal level errors that causes the program to crash.
	Logger& error();	//! log errors that stop an operation but allow the program to continue
	Logger& warn();		//! log minor errors, that will let the operation continue, but may cause unwanted behaviour. Turned off by default.
	Logger& info();		//! verbose logs that may be useful, but are optional.  !!! Prints to std::cout by default !!! Turned off by default.
	Logger& debug();	//! development warnings / messages. Those are disabled by default and should only used for development. Optimally, debug logs are remove before release.

	//! Initialize logging. Sets the streams the log-functions should write to
	void initLogging();

	//! Enable or disable specific log-levels
	void setLoggerEnabled(bool enable, LogLevel level);

	//! Enables all Log-Levels up level. E.g. WARN enables FATAL, ERROR and WARN
	//! @param level Lowest level to be enabled
	void setLogLevel(LogLevel level);

	//! Sets where the Logger should write to.
	//! @param streamObject the ostream object the logger should write to
	//! @param level the logger level to set
	void setLogStream(std::ostream* streamObject, LogLevel level);

}

#endif // LOGGING_H
