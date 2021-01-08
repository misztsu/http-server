#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <utility>
#include <string>

#define FILENAME std::string(__FILE__).substr(std::string(__FILE__).find_last_of('/') + 1)
#define DEBUG Debug() << '[' << FILENAME << __LINE__ << ']'

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
