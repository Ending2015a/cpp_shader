#pragma once
#include <iostream>
#include <algorithm>
#include <sstream>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


#ifdef DEBUG_LOG
#include <string>
#include <sstream>

int __print_step = 0;

void __pt_log(const char *h_, const char *f_, ...) {
	std::stringstream ss;
	ss << h_ << f_ << '\n';
	std::string format = ss.str();

	va_list va;
	va_start(va, f_);
	vprintf(format.c_str(), va);
	va_end(va);
	__print_step++;
}

#define VA_ARGS(...) , ##__VA_ARGS__
#define LOG_S(f_, ...) __pt_log(\
                                    "[LOG] Step %3d: ", (f_), \
                                     __print_step VA_ARGS(__VA_ARGS__))
#define LOG(f_, ...) __pt_log(\
								" [LOG]", (f_) VA_ARGS(__VA_ARGS__))

#else
#define LOG_S(f_, ...)
#define LOG(f_, ...)
#endif


#ifdef TIME_MEASURE

#define PRECISION 1000

#include <chrono>
#include <map>

using hr_clock = std::chrono::high_resolution_clock;

struct __timer {
	bool state;
	double total;
	std::chrono::time_point<hr_clock> start;
	__timer() : state(false), total(0) {}
};

std::map<std::string, struct __timer> __t_map;
inline void __ms_tic(std::string tag, bool cover = true) {
	try {
		__timer &t = __t_map[tag];
		if (!cover && t.state)
			throw std::string("the timer has already started");
		t.state = true;
		t.start = std::chrono::high_resolution_clock::now();
	}
	catch (std::string msg) {
		msg += std::string(": %s");
		LOG(msg.c_str(), tag.c_str());
	}
}

inline void __ms_toc(std::string tag, bool restart = false) {
	auto end = std::chrono::high_resolution_clock::now();
	try {
		__timer &t = __t_map[tag];
		if (!t.state)
			throw std::string("the timer is inactive");
		t.state = restart;
		std::chrono::duration<double> d = end - t.start;
		t.total += d.count() * PRECISION;
		t.start = end;
	}
	catch (std::string msg) {
		msg += std::string(": %s");
		LOG(msg.c_str(), tag.c_str());
	}
}

inline void __log_all() {
	LOG("%-30s %-30s", "Timers", "Elapsed time");
	for (auto it = __t_map.begin(); it != __t_map.end(); ++it)
		LOG("%-30s %.6lf ms", it->first.c_str(), it->second.total);
}

inline void __log_t(std::string tag) {
	LOG("%-30s %.61f ms", tag, __t_map[tag].total);
}

#define TIC(tag, ...) __ms_tic((tag))
#define TOC(tag, ...) __ms_toc((tag))
#define GET(tag) __t_map[tag].total;
#define _LOG_ALL() __log_all()
#define _LOG_TIME(tag) __log_t((tag))
#else
#define TIC(tag, ...)
#define TOC(tag, ...)
#define GET(tag) 0
#define _LOG_ALL()
#define _LOG_TIME(tag)
#endif
