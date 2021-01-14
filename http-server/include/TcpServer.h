#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <functional>

#include "Coroutine.h"
#include "EpollManager.h"
#include "TcpClient.h"

class TcpServer : public Network
{
public:

    TcpServer() = default;
    
    //TODO remake all constructors (not used atm, they dont handle callback and epollmanager)
    /*
    TcpServer(const TcpServer &) = delete;
    TcpServer(TcpServer &&rhs)
    {
        socket = rhs.socket;
        rhs.socket = invalidSocket;
    }
    TcpServer &operator =(const TcpServer &) = delete;
    TcpServer &operator =(TcpServer &&rhs)
    {
        if (socket != invalidSocket)
            close();
        socket = rhs.socket;
        rhs.socket = invalidSocket;
        return *this;
    }
    */

    using ClientTaskCallbackType = std::function<Coroutine<void>(TcpClient&&, Notify)>;

    void setClientTaskCallback(ClientTaskCallbackType&& callback)
    {
        clientTaskCallback = callback;
    }

    void bind(unsigned short port)
    {

        socket = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        // SOCK_NONBLOCK => non-blocking accept
        if (socket == invalidSocket)
            error("socket");

        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        int temp = 1;
        setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char*)&temp, sizeof(temp));

        code = ::bind(socket, (const struct sockaddr *)&addr, sizeof(addr));
        if (code == errorCode)
        {
            close();
            error("bind");
        }

        code = ::listen(socket, SOMAXCONN);
        if (code == errorCode)
        {
            close();
            error("bind");
        }

        DEBUG << "socket" << socket << "bound to port" << port << "successfully";

        addAcceptingTask();
    }

    Coroutine<TcpClient> accept()
    {
        DEBUG << "accepting on socket" << socket << "started";
        Socket clientSocket;
        do
        {
            clientSocket = ::accept(socket, nullptr, nullptr);
            DEBUG << "accept on socket" << socket << "returned" << clientSocket;
            if (tryAgain(clientSocket))
                co_await std::suspend_always();
            else
                break;
        }
        while(true);

        if (clientSocket == invalidSocket)
        {
            close();
            error("accept");
        }

        co_return TcpClient(clientSocket);
    }

    void close()
    {
        DEBUG << "close socket" << socket;
        ::close(socket);
        socket = invalidSocket;
    }

    void wait()
    {
        epollManager.wait();
    }

    [[noreturn]]
    void waitForever()
    {
        while(true)
            wait();
    }

    ~TcpServer()
    {
        if (socket != invalidSocket)
            close();
    }

private:

    void addAcceptingTask()
    {
        epollManager.addSocket(getSocket(), acceptingTask());
    }

    Coroutine<void> acceptingTask()
    {
        DEBUG << "accepting task started";
        co_await std::suspend_always();
        while (true)
        {
            auto tcpClientCoroutine = co_await accept();
            iterative_co_await(tcpClientCoroutine);
            TcpClient tcpClient = tcpClientCoroutine.value();
            Notify notify;
            Network::Socket clientSocket = tcpClient.getSocket();
            Notify::FileDescriptor efd = notify.getFileDescriptor();
            epollManager.addSocket(clientSocket, efd, clientTaskCallback(std::move(tcpClient), std::move(notify)));
        }
    }

    
    ClientTaskCallbackType clientTaskCallback;
    EpollManager epollManager;
    int code;
};

#endif /* TCPSERVER_H */
