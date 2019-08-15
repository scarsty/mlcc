#pragma once
#include <chrono>
#include <ctime>
#include <string>

//计时器类
class Timer
{
private:
    std::chrono::time_point<std::chrono::system_clock> t0_, t1_;
    bool running_ = false;

public:
    Timer() { start(); }
    ~Timer() {}

    //以字符串返回当前时间
    static std::string getNowAsString(const std::string format = "%F %a %T")
    {
        auto t = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(t);
        char buffer[80];
        strftime(buffer, 80, format.c_str(), localtime(&time));
        return buffer;
    }

    void start()
    {
        running_ = true;
        t0_ = std::chrono::system_clock::now();
    }

    void stop()
    {
        running_ = false;
        t1_ = std::chrono::system_clock::now();
    }

    double getElapsedTime()
    {
        if (running_)
        {
            t1_ = std::chrono::system_clock::now();
        }
        auto s = std::chrono::duration_cast<std::chrono::duration<double>>(t1_ - t0_);
        return s.count();
    }

    static std::string getAutoTime(double s)
    {
        const int size = 80;
        char buffer[size];
        if (s > 3600)
        {
            snprintf(buffer, size, "%.2g h", s / 3600);
        }
        else if (s > 60)
        {
            snprintf(buffer, size, "%.2g m", s / 60);
        }
        else
        {
            snprintf(buffer, size, "%.2g s", s);
        }
        return std::string(buffer);
    }
};