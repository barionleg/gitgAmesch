#ifndef LOGGER_H
#define LOGGER_H

#include <string_view>
#include <ostream>


// Logger class wrapping an ostream object. Do not use directly, use the wrapping functions in Logging.h
// Also, this logging is not thread safe! This means, that logging with multiple threads could have overlapping outputs

class Logger
{
	public:
		Logger(bool isEnabled = true);
		~Logger();

		[[nodiscard]] bool isEnabled() const;
		void setIsEnabled(bool isEnabled);

		template<typename T>
		auto operator<<(const T& msg) -> Logger&
		{
			if(mIsEnabled && mOutputStream)
				(*mOutputStream) << msg;
			return *this;
		}

		void setOutputStream(std::ostream* outputStream);
	private:
		std::ostream* mOutputStream;
		bool mIsEnabled = true;
};

#endif // LOGGER_H
