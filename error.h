#ifndef ERROR_H
#define ERROR_H

#include <cstring>
#include <stdexcept>
#include "Debug.h"

template <class T>
static void error(T &&message)
{
    Debug() << message << errno << std::strerror(errno);
    throw std::runtime_error(std::forward<T>(message));
}

#endif /* ERROR_H */
