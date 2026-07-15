#pragma once

#include <chrono>
#include <ctime>
#include <string>

#include "runtime_format.h"

class Timer
{
private:
    std::chrono::time_point<std::chrono::system_clock> t0_, t1_, t_last_;
    bool running_ = false;

public:
    Timer() { start(); }

    // Returns current time as a string
    static std::string getNowAsString(const std::string& format = "%F %a %H:%M:%S", bool with_ms = false)
    {
        //#ifdef __cpp_lib_format
        //        auto t = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
        //        auto t1 = std::chrono::current_zone()->to_local(t);
        //        auto fmt = "{:" + format + "}";
        //        return std::vformat(fmt, std::make_format_args(t1));
        //#else
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = {};
    #ifdef _WIN32
        localtime_s(&tm, &t);
    #else
        localtime_r(&t, &tm);
    #endif
        char buffer[64] = {};
        strftime(buffer, sizeof(buffer), format.c_str(), &tm);
        auto str = std::string(buffer);
        if (with_ms)
        {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            str += std::format(std::runtime_format{".{:03}"}, ms.count());
        }
        return str;
        //#endif
    }

    void start()
    {
        running_ = true;
        t0_ = std::chrono::system_clock::now();
        t_last_ = t0_;
    }

    void stop()
    {
        running_ = false;
        t1_ = std::chrono::system_clock::now();
    }

    double getElapsedTime(bool restart = false)
    {
        if (running_)
        {
            t1_ = std::chrono::system_clock::now();
        }
        auto s = std::chrono::duration_cast<std::chrono::duration<double>>(t1_ - t0_);
        if (restart)
        {
            t0_ = std::chrono::system_clock::now();
        }
        return s.count();
    }

    double getLastPeriod()
    {
        return getElapsedTime(true);
    }

    static std::string autoFormatTime(double s)
    {
        auto remaining = std::chrono::duration<double>{s};
        const auto hours = std::chrono::duration_cast<std::chrono::hours>(remaining);
        remaining -= hours;
        const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(remaining);
        remaining -= minutes;
        const auto seconds = remaining.count();
        if (hours.count() > 0)
        {
            return std::format(std::runtime_format{"{}:{:02}:{:05.2f}"}, hours.count(), minutes.count(), seconds);
        }
        if (minutes.count() > 0)
        {
            return std::format(std::runtime_format{"{}:{:05.2f}"}, minutes.count(), seconds);
        }
        return std::format(std::runtime_format{"{:.2f} s"}, seconds);
    }

    static std::string formatTime(double s, const std::string& format_str = "{}:{:02}:{:05.2f}")
    {
        auto remaining = std::chrono::duration<double>{s};
        const auto hours = std::chrono::duration_cast<std::chrono::hours>(remaining);
        remaining -= hours;
        const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(remaining);
        remaining -= minutes;
        return std::format(std::runtime_format{format_str}, hours.count(), minutes.count(), remaining.count());
    }

    static int64_t getNanoTime()
    {
        return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }

    // return the time within two callings of getLastPeriod2, no restart
    double getLastPeriod2()
    {
        if (running_)
        {
            t1_ = std::chrono::system_clock::now();
        }
        auto s = std::chrono::duration_cast<std::chrono::duration<double>>(t1_ - t_last_);
        t_last_ = t1_;
        return s.count();
    }
};