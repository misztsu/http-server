#include <future>

#include "TcpServer.h"
#include "EpollManager.h"

Coroutine<void> clientHandlingTask(TcpClient &&client, Notify notify)
{
    TcpClient tcpClient = std::move(client);
    co_await std::suspend_always();

    auto message = tcpClient.receive();
    iterative_co_await(message);
    Debug() << "received message";

    if (message.value().size() == 0)
        co_return;

    auto response = std::async(std::launch::async, [](Notify notify) {
        auto response = "HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=utf-8\r\n\r\nHello";
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // simulate blocking operation

        return response;
    }, std::move(notify));
    co_await std::suspend_always();

    auto sent = tcpClient.send(response.get());
    iterative_co_await(sent);

    tcpClient.close();
}

Coroutine<void> acceptingTask(TcpServer &&tcpServer, EpollManager &epollManager)
{
    co_await std::suspend_always();
    while (true)
    {
        TcpClient tcpClient = (co_await tcpServer.accept()).value();
        Notify notify;
        Network::Socket clientSocket = tcpClient.getSocket();
        Notify::FileDescriptor efd = notify.getFileDescriptor();
        epollManager.addSocket(clientSocket, efd, clientHandlingTask(std::move(tcpClient), std::move(notify)));
    }
}

int main()
{
    EpollManager epollManager;

    TcpServer tcpServer;
    tcpServer.bind(3000);

    epollManager.addSocket(tcpServer.getSocket(), acceptingTask(std::move(tcpServer), epollManager));

    while (true)
        epollManager.wait();

    return 0;
}