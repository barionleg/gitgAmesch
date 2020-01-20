#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>

namespace timer {
class Timer {
      private:
	static std::unordered_map<
	    std::string, std::chrono::time_point<std::chrono::system_clock>>
	    start_times;
	static std::unordered_map<
	    std::string, double>
	    run_times;

      public:
	static void start(const std::string &action);
	static void stop(const std::string &action);
	static double get(const std::string &action);
};

} // namespace utility

#endif