#ifndef EPOLLMANAGER_H
#define EPOLLMANAGER_H

#include <unordered_map>
#include "Network.h"
#include "Coroutine.h"

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
        Debug() << "adding socket" << socket << "to epoll manager";
        epoll_event event;
        event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET;
        event.data.fd = socket;
        if (epoll_ctl(epollFileDescriptor, EPOLL_CTL_ADD, socket, &event) == errorCode)
            error("epoll_ctl");

        task.resume();

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
        Debug() << "waiting";
        epoll_event event;
        code = epoll_wait(epollFileDescriptor, &event, 1, -1);
        if (code == errorCode)
            error("epoll_wait");

        if ((event.events & EPOLLIN) == EPOLLIN)
            Debug() << "EPOLLIN event on socket" << (int)event.data.fd;

        if ((event.events & EPOLLOUT) == EPOLLOUT)
            Debug() << "EPOLLOUT event on socket" << (int)event.data.fd;

        if ((event.events & EPOLLRDHUP) == EPOLLRDHUP)
            Debug() << "EPOLLRDHUP event on socket" << (int)event.data.fd;

        auto &task = tasks.at(event.data.fd);
        if (!task)
            task.resume();
        else
            throw std::runtime_error("there is finished task not removed from epoll manager");
    }

private:
    int epollFileDescriptor = -1;
    static constexpr int errorCode = -1;
    int code = errorCode;
    std::unordered_map<Network::Socket, Coroutine<void>> tasks;
};

#endif /* EPOLLMANAGER_H */
