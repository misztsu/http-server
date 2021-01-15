#ifndef DEBUG_H
#define DEBUG_H

#include <filesystem>
#include <iostream>
#include <utility>
#include <string>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define DEBUG Debug() << '[' << std::filesystem::path(__FILE__).filename().string() + ":" TOSTRING(__LINE__) << ']'

class Debug
{
public:
    Debug() = default;
    Debug(Debug &&) = default;

    template <class T>
    Debug &operator<<(T &&t)
    {
        std::cout << std::forward<T>(t) << ' ';
        return *this;
    }

    ~Debug()
    {
        std::cout << std::endl;
    }

    static void pushColor(uint8_t color)
    {
        std::cout << "\033[38;5;" << +color << "m";
        colors.push(color);
    }
    static void popColor()
    {
        if (!colors.empty())
            colors.pop();
        if (!colors.empty())
            std::cout << "\033[38;5;" << +colors.top() << "m";
        else
            std::cout << "\033[0m";
    }

private:
    static inline std::stack<uint8_t> colors;
};

#endif /* DEBUG_H */
