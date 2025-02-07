/* TermTk
 * Simple, low level Terminal toolkit for C++
 *
 * Copyright (C) 2024-2025 Jacob Nilsson
 * Licensed under the MIT license
 * https://git.jacobnilsson.com/TermTk
 */
#pragma once

#define TT_VERSION_MAJOR 0
#define TT_VERSION_MINOR 1
#define TT_VERSION_PATCH 0
#define TT_UNRELEASED 1

/* Platform detection */
#ifdef _WIN32
    #define TT_PLATFORM_WINDOWS
    #define TT_PLATFORM "Windows"
#elif __APPLE__
    #define TT_PLATFORM_APPLE
    #define TT_PLATFORM "Apple"
#elif __linux__
    #define TT_PLATFORM_LINUX
    #define TT_PLATFORM "Linux"
#elif __unix__
    #define TT_PLATFORM_UNIX
    #define TT_PLATFORM "Unix"
#else
    #define TT_PLATFORM_UNKNOWN
    #define TT_PLATFORM "Unknown"
#endif

#include <string_view>
#include <iostream>
#include <sstream>
#include <thread>

/* system includes */
#include <sys/ioctl.h> // for ioctl
#include <termios.h> // for termios
#include <unistd.h> // for isatty

namespace _TermTk_Internal {
    static bool th_key{false};
    void get_key();
}

namespace TermTk {
    using Integer = int;
    using Bool = bool;
    using String = std::string;
    using StringView = std::string_view;

    struct Size {
        Integer rows{};
        Integer cols{};
    };

    struct CursorPosition {
        Integer x{};
        Integer y{};
    };

    enum class Key { Up, Down, Left, Right, Enter, Backspace, Delete, Tab, Escape, Space, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, Home, End, PageUp, PageDown, Insert, Unknown };

    namespace Color::Foreground {
        static constexpr StringView None;
        static constexpr StringView Black = "\033[30m";
        static constexpr StringView Red = "\033[31m";
        static constexpr StringView Green = "\033[32m";
        static constexpr StringView Yellow = "\033[33m";
        static constexpr StringView Blue = "\033[34m";
        static constexpr StringView Magenta = "\033[35m";
        static constexpr StringView Cyan = "\033[36m";
        static constexpr StringView White = "\033[37m";

        static constexpr StringView BrightBlack = "\033[90m";
        static constexpr StringView BrightRed = "\033[91m";
        static constexpr StringView BrightGreen = "\033[92m";
        static constexpr StringView BrightYellow = "\033[93m";
        static constexpr StringView BrightBlue = "\033[94m";
        static constexpr StringView BrightMagenta = "\033[95m";
        static constexpr StringView BrightCyan = "\033[96m";
        static constexpr StringView BrightWhite = "\033[97m";

        StringView get(const String& hex);
    };

    namespace Color::Background {
        static constexpr StringView None;
        static constexpr StringView Black = "\033[40m";
        static constexpr StringView Red = "\033[41m";
        static constexpr StringView Green = "\033[42m";
        static constexpr StringView Yellow = "\033[43m";
        static constexpr StringView Blue = "\033[44m";
        static constexpr StringView Magenta = "\033[45m";
        static constexpr StringView Cyan = "\033[46m";
        static constexpr StringView White = "\033[47m";

        static constexpr StringView BrightBlack = "\033[100m";
        static constexpr StringView BrightRed = "\033[101m";
        static constexpr StringView BrightGreen = "\033[102m";
        static constexpr StringView BrightYellow = "\033[103m";
        static constexpr StringView BrightBlue = "\033[104m";
        static constexpr StringView BrightMagenta = "\033[105m";
        static constexpr StringView BrightCyan = "\033[106m";
        static constexpr StringView BrightWhite = "\033[107m";

        StringView get(const String& hex);
    }

    namespace Cursor {
        static constexpr StringView Up = "\033[A";
        static constexpr StringView Down = "\033[B";
        static constexpr StringView Left = "\033[D";
        static constexpr StringView Right = "\033[C";
        static constexpr StringView Hide = "\033[?25l";
        static constexpr StringView Show = "\033[?25h";
    }

    namespace Font {
        static constexpr StringView Bold = "\033[1m";
    }

    namespace Sequence {
        static constexpr StringView Reset = "\033[0m";
        static constexpr StringView ClearScreen = "\033[2J";
        static constexpr StringView CursorPosition = "\033[%d;%dH";
        static constexpr StringView EraseLine = "\033[K";
    }

    Bool supports_24bit();
    Integer closest_color(Integer r, Integer g, Integer b);
    Size get_size();
    void clear_screen();
    void set_cursor(Integer x, Integer y);
    void set_cursor(const CursorPosition& pos);
    void set_cursor(Bool show);
    void print_one_line(const String& str);
    void fill_entire_background(const String& color);
    void set_raw_mode(Bool enable);
    void finalize();
    Key get_key();
    String get_input();
    void sleep(Integer ms);

    static Key key{Key::Unknown};
    static String input;
}

TermTk::Integer TermTk::closest_color(TermTk::Integer r, TermTk::Integer g, TermTk::Integer b) {
    static constexpr std::array<std::array<TermTk::Integer, 3>, 8> color_palette = {{
        {0, 0, 0}, {128, 0, 0}, {0, 128, 0}, {128, 128, 0}, {0, 0, 128}, {128, 0, 128}, {0, 128, 128}, {192, 192, 192}
    }};

    TermTk::Integer closest{0};
    double min_distance = std::numeric_limits<double>::max();

    for (TermTk::Integer i = 0; i < 8; ++i) {
        double distance = std::sqrt(
            std::pow(r - color_palette[i][0], 2) +
            std::pow(g - color_palette[i][1], 2) +
            std::pow(b - color_palette[i][2], 2)
        );

        if (distance < min_distance) {
            min_distance = distance;
            closest = i;
        }
    }

    return closest;
}

TermTk::StringView TermTk::Color::Foreground::get(const TermTk::String& hex) {
    if (hex.size() != 7 || hex[0] != '#') {
        return Sequence::Reset;
    }

    TermTk::Integer r, g, b;
    std::istringstream(hex.substr(1, 2)) >> std::hex >> r;
    std::istringstream(hex.substr(3, 2)) >> std::hex >> g;
    std::istringstream(hex.substr(5, 2)) >> std::hex >> b;

    static std::string sgr;
    std::ostringstream oss;

    if (!supports_24bit()) {
        TermTk::Integer closest = closest_color(r, g, b);
        oss << "\033[" << 30 + closest << "m";
    } else {
        oss << "\033[38;2;" << r << ";" << g << ";" << b << "m";
    }

    sgr = oss.str();
    return sgr;
}

TermTk::StringView TermTk::Color::Background::get(const TermTk::String& hex) {
    if (hex.size() != 7 || hex[0] != '#') {
        return Sequence::Reset;
    }

    TermTk::Integer r, g, b;
    std::istringstream(hex.substr(1, 2)) >> std::hex >> r;
    std::istringstream(hex.substr(3, 2)) >> std::hex >> g;
    std::istringstream(hex.substr(5, 2)) >> std::hex >> b;

    static std::string sgr;
    std::ostringstream oss;

    if (!supports_24bit()) {
        TermTk::Integer closest = closest_color(r, g, b);
        oss << "\033[" << 40 + closest << "m";
    } else {
        oss << "\033[48;2;" << r << ";" << g << ";" << b << "m";
    }

    sgr = oss.str();
    return sgr;
}

TermTk::Size TermTk::get_size() {
    struct winsize w{};
    ioctl(0, TIOCGWINSZ, &w);
    return {w.ws_row, w.ws_col};
}

TermTk::Bool TermTk::supports_24bit() {
    const char* colorterm = std::getenv("COLORTERM");
    if (colorterm != nullptr) {
        std::string colorterm_str = colorterm;
        return colorterm_str == "truecolor" || colorterm_str == "24bit";
    }
    return false;
}

void TermTk::clear_screen() {
    std::cout << "\033[2J\033[1;1H";
}

void TermTk::set_cursor(TermTk::Integer x, TermTk::Integer y) {
    std::cout << "\033[" << (x + 1) << ";" << (y + 1) << "H" << std::flush;
}

void TermTk::set_cursor(const CursorPosition& pos) {
    set_cursor(pos.x, pos.y);
}

void TermTk::set_cursor(TermTk::Bool show) {
    std::cout << (show ? Cursor::Show : Cursor::Hide);
}

void TermTk::print_one_line(const TermTk::String& str) {
    auto size = get_size();

    if (str.size() > size.cols) {
        std::cout << str.substr(0, size.cols);
    } else {
        std::cout << str;
    }
}

void TermTk::fill_entire_background(const TermTk::String &color) {
    auto size = get_size();
    String fill_line = color + std::string(size.cols, ' ') + std::string(Sequence::Reset);
    for (TermTk::Integer i = 0; i < size.rows + 1; ++i) {
        set_cursor(i, 0);
        std::cout << fill_line << std::flush;
    }
    set_cursor(0, 0);
}


void TermTk::set_raw_mode(const TermTk::Bool enable) {
    static termios oldt;
    if (enable) {
        termios newt{};
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

/* Be in a while (running) loop before calling this function */
TermTk::Key TermTk::get_key() {
    _TermTk_Internal::get_key();
    return key;
}

/* Be in a while (running) loop before calling this function */
TermTk::String TermTk::get_input() {
    _TermTk_Internal::get_key();
    return input;
}

/* To avoid confusion, call this when you're done drawing */
void TermTk::finalize() {
    std::cout << std::flush;
}

void TermTk::sleep(TermTk::Integer ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void _TermTk_Internal::get_key() {
    TermTk::set_raw_mode(true);
    if (th_key) {
      return;
    }

std::thread t([]() {
    th_key = true;
    while (true) {
        char c{};
        std::cin.read(&c, 1);

        if (c == '\033') {
            std::vector<char> buffer(2);
            std::cin.read(buffer.data(), 2);

            if (buffer.at(0) == '[') {
                switch (buffer.at(1)) {
                    case 'A': TermTk::key = TermTk::Key::Up; break;
                    case 'B': TermTk::key = TermTk::Key::Down; break;
                    case 'C': TermTk::key = TermTk::Key::Right; break;
                    case 'D': TermTk::key = TermTk::Key::Left; break;
                    case 'H': TermTk::key = TermTk::Key::Home; break;
                    case 'F': TermTk::key = TermTk::Key::End; break;
                    case '5': std::cin.read(&c, 1); TermTk::key = c == '~' ? TermTk::Key::PageUp : TermTk::Key::Unknown; break;
                    case '6': std::cin.read(&c, 1); TermTk::key = c == '~' ? TermTk::Key::PageDown : TermTk::Key::Unknown; break;
                    case '2': std::cin.read(&c, 1); TermTk::key = c == '~' ? TermTk::Key::Insert : TermTk::Key::Unknown; break;
                    case '3': std::cin.read(&c, 1); TermTk::key = c == '~' ? TermTk::Key::Delete : TermTk::Key::Unknown; break;
                    default: TermTk::key = TermTk::Key::Unknown; break;
                }
            } else if (buffer.at(0) == 'O') {
                switch (buffer.at(1)) {
                    case 'P': TermTk::key = TermTk::Key::F1; break;
                    case 'Q': TermTk::key = TermTk::Key::F2; break;
                    case 'R': TermTk::key = TermTk::Key::F3; break;
                    case 'S': TermTk::key = TermTk::Key::F4; break;
                    case 'A': TermTk::key = TermTk::Key::F5; break;
                    case 'B': TermTk::key = TermTk::Key::F6; break;
                    case 'C': TermTk::key = TermTk::Key::F7; break;
                    case 'D': TermTk::key = TermTk::Key::F8; break;
                    case 'E': TermTk::key = TermTk::Key::F9; break;
                    case 'F': TermTk::key = TermTk::Key::F10; break;
                    case 'G': TermTk::key = TermTk::Key::F11; break;
                    case 'H': TermTk::key = TermTk::Key::F12; break;
                    default: TermTk::key = TermTk::Key::Unknown; break;
                }
            } else {
                TermTk::key = TermTk::Key::Unknown;
            }
        } else if (c == 127 || c == '\b') {
            TermTk::key = TermTk::Key::Backspace;
        } else if (c == '\n') {
            TermTk::key = TermTk::Key::Enter;
            TermTk::input.clear();
        } else if (c == 27) {
            TermTk::key = TermTk::Key::Escape;
        } else {
            TermTk::input += c;
            TermTk::key = TermTk::Key::Unknown;
        }
    }
});

    t.detach();
}

#if TT_UNRELEASED
#ifndef TT_ALLOW_UNRELEASED
#warning "TermTk is in an unreleased state and is not intended for production use"
#endif
#endif