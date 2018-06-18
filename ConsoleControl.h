#pragma once
#include <map>
#include <string>

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
    ConsoleControl();
    virtual ~ConsoleControl();

    static ConsoleControl console_control_;

    unsigned short old_color_;
    std::map<int, std::string> color_map_;

    static ConsoleControl* instance() { return &console_control_; }

public:
    static void setColor(int c);
    static void moveUp(int l = 1);
};
