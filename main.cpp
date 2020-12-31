#include "TcpServer.h"
#include "EpollManager.h"

Coroutine<void> clientHandlingTask(TcpClient &&client)
{
    TcpClient tcpClient = std::move(client);
    co_await std::suspend_always();

    auto message = tcpClient.receive();
    iterative_co_await(message);
    Debug() << "received message";

    if (message.value().size() == 0)
        co_return;

    auto sent = tcpClient.send("HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=utf-8\r\n\r\nHello");
    iterative_co_await(sent);

    tcpClient.close();
}

Coroutine<void> acceptingTask(TcpServer &&tcpServer, EpollManager &epollManager)
{
    co_await std::suspend_always();
    while (true)
    {
        TcpClient tcpClient = (co_await tcpServer.accept()).value();
        Network::Socket clientSocket = tcpClient.getSocket();
        epollManager.addSocket(clientSocket, clientHandlingTask(std::move(tcpClient)));
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