#include "ConsoleControl.h"
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else

#endif

ConsoleControl ConsoleControl::console_control_;

ConsoleControl::ConsoleControl()
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

ConsoleControl::~ConsoleControl()
{
}

void ConsoleControl::setColor(int c)
{
#ifdef _MSC_VER
    if (c != CONSOLE_COLOR_NONE)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
    }
    else
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), console_control_.old_color_);
    }
#else
    fprintf(stdout, console_control_.color_map_[c].c_str());
#endif
}

void ConsoleControl::moveUp(int l)
{
#ifdef _MSC_VER
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    info.dwCursorPosition.Y -= l;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), info.dwCursorPosition);
#else
    fprintf(stdout, "\e[%dA", l);
#endif
}
