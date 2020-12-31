#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "TcpClient.h"

class TcpServer : public Network
{
public:
    TcpServer() = default;
    
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

        Debug() << "socket" << socket << "bound to port" << port << "successfully";
    }

    Coroutine<TcpClient> accept()
    {
        Debug() << "accepting on socket" << socket << "started";
        Socket clientSocket;
        do
        {
            clientSocket = ::accept(socket, nullptr, nullptr);
            Debug() << "accept on socket" << socket << "returned" << clientSocket;
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
        Debug() << "close socket" << socket;
        ::close(socket);
        socket = invalidSocket;
    }

    ~TcpServer()
    {
        if (socket != invalidSocket)
            close();
    }

private:
    int code;
};

#endif /* TCPSERVER_H */
