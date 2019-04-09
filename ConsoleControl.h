#pragma once
#include <map>
#include <stdio.h>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#else
#include <stdio.h>
#endif

enum ConsoleColor
{
    CONSOLE_COLOR_NONE = -1,
    CONSOLE_COLOR_RED = 4,
    CONSOLE_COLOR_LIGHT_RED = 12,
    CONSOLE_COLOR_GREEN = 2,
    CONSOLE_COLOR_LIGHT_GREEN = 10,
    CONSOLE_COLOR_BLUE = 1,
    CONSOLE_COLOR_LIGHT_BLUE = 9,
    CONSOLE_COLOR_WHITE = 7,
    CONSOLE_COLOR_BLACK = 0,
};

class ConsoleControl
{
private:
    ConsoleControl()
    {
        if (color_map_.empty())
        {
            color_map_ =
            {
                { CONSOLE_COLOR_NONE, "\e[0m" },
                { CONSOLE_COLOR_RED, "\e[0;31m" },
                { CONSOLE_COLOR_LIGHT_RED, "\e[1;31m" },
                { CONSOLE_COLOR_GREEN, "\e[0;32m" },
                { CONSOLE_COLOR_LIGHT_GREEN, "\e[1;32m" },
                { CONSOLE_COLOR_BLUE, "\e[0;34m" },
                { CONSOLE_COLOR_LIGHT_BLUE, "\e[1;34m" },
                { CONSOLE_COLOR_WHITE, "\e[1;37m" },
                { CONSOLE_COLOR_BLACK, "\e[0;30m" },
            };
#ifdef _WIN32
            CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbiInfo);
            old_color_ = csbiInfo.wAttributes;
#endif
        }
    }
    virtual ~ConsoleControl()
    {
    }

    unsigned short old_color_;
    std::map<int, std::string> color_map_;

    static ConsoleControl* getInstance()
    {
        static ConsoleControl console_control;
        return &console_control;
    }

private:
    ConsoleControl(ConsoleControl&) = delete;
    ConsoleControl& operator=(ConsoleControl&) = delete;

public:
    static void setColor(int c)
    {
        auto cc = getInstance();
#ifdef _MSC_VER
        if (c != CONSOLE_COLOR_NONE)
        {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
        }
        else
        {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), cc->old_color_);
        }
#else
        fprintf(stderr, "%s", cc->color_map_[c].c_str());
#endif
    }
    static void moveUp(int l = 1)
    {
#ifdef _MSC_VER
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
        info.dwCursorPosition.Y -= l;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), info.dwCursorPosition);
#else
        fprintf(stderr, "\e[%dA", l);
#endif
    }
};
