#include "timer.h"

using namespace timer;

std::unordered_map<std::string,
		   std::chrono::time_point<std::chrono::system_clock>>
    Timer::start_times;

std::unordered_map<std::string, double> Timer::run_times;

void Timer::start(const std::string &action) {
		Timer::start_times[action] = std::chrono::system_clock::now();
	}

void Timer::stop(const std::string &action) {
    auto now = std::chrono::system_clock::now();
    auto start_time_it = Timer::start_times.find(action);
    if (start_time_it != Timer::start_times.end()) {
        std::chrono::duration<double> elapsed_seconds =
        now - start_time_it->second;
        Timer::run_times[action] = elapsed_seconds.count();
    }
}

double Timer::get(const std::string &action) {
    return Timer::run_times[action];
}