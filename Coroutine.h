#ifndef COROUTINE_H
#define COROUTINE_H

#include <coroutine>
#include <exception>
#include <type_traits>
#include <optional>

#include "Debug.h"

template <class T>
struct Promise
{
    auto get_return_object() { return std::coroutine_handle<Promise>::from_promise(*this); }
    auto initial_suspend() { return std::suspend_never(); }
    auto final_suspend() { return std::suspend_always(); }
    void unhandled_exception()
    {
        auto exceptionPtr = std::current_exception();
        if (exceptionPtr)
            std::rethrow_exception(exceptionPtr);
    }
    template <class U>
    void return_value(U &&u) { optional = std::forward<U>(u); }
    bool hasValue() const { return optional.has_value(); }
    T value()
    {
        T value = std::move(*optional);
        optional.reset();
        return value;
    }
    std::optional<T> optional;
};

template <>
struct Promise<void>
{
    auto get_return_object() { return std::coroutine_handle<Promise>::from_promise(*this); }
    auto initial_suspend() { return std::suspend_never(); }
    auto final_suspend() { return std::suspend_always(); }
    void unhandled_exception()
    {
        auto exceptionPtr = std::current_exception();
        if (exceptionPtr)
            std::rethrow_exception(exceptionPtr);
    }
    constexpr void return_void() noexcept {}
    bool hasValue() const { return false; }
    constexpr void value() noexcept {}
};

template <class T>
class Coroutine
{
public:
    using promise_type = Promise<T>;

    Coroutine(std::coroutine_handle<promise_type> handle) : coroutineHandle(handle) {}
    Coroutine(const Coroutine &) = delete;
    Coroutine(Coroutine &&rhs) noexcept : coroutineHandle(rhs.coroutineHandle)
    {
        rhs.coroutineHandle = {};
    }

    Coroutine &operator=(const Coroutine &) = delete;
    Coroutine &operator=(Coroutine &&rhs) noexcept
    {
        if (this != &rhs)
        {
            if (coroutineHandle)
                coroutineHandle.destroy();
            coroutineHandle = rhs.coroutineHandle;
            rhs.coroutineHandle = {};
        }
        return *this;
    }

    bool resume() const
    {
        Debug() << "resume coroutine";
        if (!done())
            coroutineHandle.resume();
        return !done();
    }

    bool done() const
    {
        return coroutineHandle && coroutineHandle.done();
    }

    operator bool() const
    {
        return done();
    }

    ~Coroutine()
    {
        if (coroutineHandle)
            coroutineHandle.destroy();
    }

    bool hasValue() const
    {
        return coroutineHandle && coroutineHandle.promise().hasValue();
    }

    T value() const
    {
        if (hasValue())
        {
            return coroutineHandle.promise().value();
        }
        else
        {
            Debug() << "coroutine has no return value";
            throw std::runtime_error("coroutine has no return value");
        }
    }

    bool await_ready() const
    {
        return done();
    }

    constexpr void await_suspend(std::coroutine_handle<>) const noexcept { }

    Coroutine<T> await_resume()
    {
        resume();
        return std::move(*this);
    }

private:
    std::coroutine_handle<promise_type> coroutineHandle;
};

// Totally unnecessary syntactic sugar
#define iterative_co_await(coroutine)       \
{                                           \
    while (!coroutine)                      \
        coroutine = co_await coroutine;     \
}   

#endif /* COROUTINE_H */
