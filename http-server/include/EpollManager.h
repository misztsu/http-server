#ifndef EPOLLMANAGER_H
#define EPOLLMANAGER_H

#include <unordered_map>
#include <sys/eventfd.h>

#include "Network.h"
#include "Coroutine.h"

class Notify
{
public:
    using FileDescriptor = int;
    Notify() : efd(eventfd(0, 0))
    {
        if (efd == invalid)
            error("Notify");
    }

    Notify(const Notify &) = delete;
    Notify(Notify &&rhs)
    {
        efd = rhs.efd;
        rhs.efd = invalid;
    }

    Notify &operator=(const Notify &) = delete;
    Notify &operator=(Notify &&rhs)
    {
        if (efd != invalid)
            operator()();
        efd = rhs.efd;
        rhs.efd = invalid;
        return *this;
    }

    FileDescriptor getFileDescriptor() const
    {
        return efd;
    }

    ~Notify()
    {
        if (efd != invalid)
            operator()();
    }

    static constexpr FileDescriptor invalid = -1;

    void operator()() const
    {
        DEBUG << "notify eventfd" << efd;
        uint64_t buff = 1;
        if (write(efd, &buff, sizeof(uint64_t)) != sizeof(uint64_t))
            error("notify");
    }

    void release()
    {
        efd = invalid;
    }

    Notify createCopy()
    {
        return Notify(efd);
    }

private:
    Notify(int efd) : efd(efd) {}

    static void close(FileDescriptor efd)
    {
        if (efd != invalid)
            if (::close(efd) == errorCode)
                error("close");
    }

    FileDescriptor efd;
    friend class EpollManager;
};

class EpollManager
{
public:
    EpollManager()
    {
        epollFileDescriptor = epoll_create1(EPOLL_CLOEXEC);
        if (epollFileDescriptor == errorCode)
            error("epoll_create1");
    }

    ~EpollManager()
    {
        if (epollFileDescriptor != errorCode)
            close(epollFileDescriptor);
    }

    void addSocket(Network::Socket socket, Coroutine<void> &&task)
    {
        addSocket(socket, Notify::invalid, std::move(task));
    }

    void addSocket(Network::Socket socket, Notify::FileDescriptor efd, Coroutine<void> &&task)
    {
        DEBUG << "adding socket" << socket << "to epoll manager";
        epoll_event event;

        event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET;
        event.data.u64 = Descriptors(socket, efd).toUint64();

        if (epoll_ctl(epollFileDescriptor, EPOLL_CTL_ADD, socket, &event) == errorCode)
            error("epoll_ctl");

        if (efd != Notify::invalid)
        {
            event.events = EPOLLIN | EPOLLET;
            if (epoll_ctl(epollFileDescriptor, EPOLL_CTL_ADD, efd, &event) == errorCode)
                error("epoll_ctl");
        }

        task.resume();
        if (task.hasValue())
            task.value();

        auto it = tasks.find(socket);
        if (it != tasks.end())
        {
            if (!it->second)
                throw std::runtime_error("not finished task overriden in epoll manager");
            it->second = std::move(task);
        }
        else
            tasks.emplace(socket, std::move(task));
    }

    void wait()
    {
        DEBUG << "waiting";
        epoll_event event;
        code = epoll_wait(epollFileDescriptor, &event, 1, -1);
        if (code == errorCode)
            error("epoll_wait");

        Descriptors descriptors = Descriptors::fromUint64(event.data.u64);

        if ((event.events & EPOLLIN) == EPOLLIN)
            DEBUG << "EPOLLIN event on fd" << descriptors.socket << "or" << descriptors.efd;

        if ((event.events & EPOLLOUT) == EPOLLOUT)
            DEBUG << "EPOLLOUT event on fd" << descriptors.socket;

        if ((event.events & EPOLLRDHUP) == EPOLLRDHUP)
            DEBUG << "EPOLLRDHUP event on fd" << descriptors.socket;

        auto it = tasks.find(descriptors.socket);
        if (!it->second)
        {
            DEBUG << "Resuming task for descriptor" << descriptors.socket;
            it->second.resume();
            if (it->second.hasValue())
                it->second.value();
                
            if (it->second)
            {
                DEBUG << "socket" << it->first << "erased from epoll manager";
                tasks.erase(it);
                Notify::close(descriptors.efd);
            }
        }
        else
            throw std::runtime_error("there is finished task not removed from epoll manager");
    }

private:
    struct Descriptors
    {
        Network::Socket socket;
        Notify::FileDescriptor efd;

        Descriptors() = default;
        Descriptors(Network::Socket s, Notify::FileDescriptor e)
            : socket(s), efd(e) {}

        static Descriptors fromUint64(uint64_t u64)
        {
            static_assert(sizeof(Descriptors) == sizeof(uint64_t));
            Descriptors descriptors;
            std::memcpy(&descriptors, &u64, sizeof(Descriptors));
            return descriptors;
        }

        uint64_t toUint64() const
        {
            static_assert(sizeof(Descriptors) == sizeof(uint64_t));
            return *reinterpret_cast<const uint64_t *>(this);
        }
    };

    int epollFileDescriptor = -1;
    int code = errorCode;
    std::unordered_map<Network::Socket, Coroutine<void>> tasks;
};

#endif /* EPOLLMANAGER_H */
