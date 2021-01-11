#ifndef DEBUG_H
#define DEBUG_H

#include <filesystem>
#include <iostream>
#include <utility>
#include <string>

#define DEBUG Debug() << '[' << std::filesystem::path(__FILE__).filename().string() << __LINE__ << ']'

class Debug
{
public:
    Debug() = default;
    Debug(Debug &&) = default;

    template <class T>
    Debug &operator<<(T &&t)
    {
        std::cerr << std::forward<T>(t) << ' ';
        return *this;
    }

    ~Debug()
    {
        std::cerr << std::endl;
    }
};

#endif /* DEBUG_H */
