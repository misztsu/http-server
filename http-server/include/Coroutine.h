#ifndef COROUTINE_H
#define COROUTINE_H

#include <coroutine>
#include <exception>
#include <type_traits>
#include <future>
#include <chrono>
#include <random>

#include "Debug.h"

template <class T>
struct Promise
{
    auto get_return_object() { return std::coroutine_handle<Promise>::from_promise(*this); }
    auto initial_suspend()
    {
        DEBUG << "starting coroutine";
        resetColor();
        return std::suspend_never();
    }
    auto final_suspend() { return std::suspend_always(); }
    void unhandled_exception()
    {
        DEBUG << "ending coroutine";
        resetColor();
        promise.set_exception(std::current_exception());
    }
    template <class U>
    void return_value(U &&u)
    {
        if constexpr (!std::is_same_v<T, void>)
        {
            DEBUG << "ending coroutine";
            resetColor();
            promise.set_value(std::forward<U>(u));
        }
    }
    void return_void()
    {
        if constexpr (std::is_same_v<T, void>)
        {
            DEBUG << "ending coroutine";
            resetColor();
            promise.set_value();
        }
    }
    bool hasValue() const { return future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready; }
    T value() { return future.get(); }
    static uint8_t randomColor()
    {
        static std::random_device rd;
        static std::mt19937 mt(rd());
        static std::uniform_int_distribution<uint8_t> random(17, 231);
        return random(mt);
    }
    void setColor() const { Debug::pushColor(color); }
    static void resetColor() { Debug::popColor(); }
    uint8_t color = randomColor();
    std::promise<T> promise;
    std::future<T> future = promise.get_future();
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
        if (!done())
        {
            coroutineHandle.promise().setColor();
            DEBUG << "resuming coroutine";
            coroutineHandle.resume();
            coroutineHandle.promise().resetColor();
        }
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
            DEBUG << "coroutine has no return value";
            throw std::runtime_error("coroutine has no return value");
        }
    }

    bool await_ready() const
    {
        return hasValue();
    }

    constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}

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
