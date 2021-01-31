#ifndef TcpClient_H
#define TcpClient_H

#include "Network.h"

class TcpClient : public Network
{
public:
    TcpClient() = default;

    TcpClient(const TcpClient &) = delete;
    TcpClient(TcpClient &&rhs)
    {
        socket = rhs.socket;
        rhs.socket = invalidSocket;
    }

    TcpClient &operator=(const TcpClient &) = delete;
    TcpClient &operator=(TcpClient &&rhs)
    {
        if (socket != invalidSocket)
            close();
        socket = rhs.socket;
        rhs.socket = invalidSocket;
        return *this;
    }

    void connect(const std::string &address, unsigned short port)
    {
        socket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (socket == invalidSocket)
        {
            error("socket");
        }

        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

        code = ::connect(socket, (const struct sockaddr *)&addr, sizeof(addr));
        if (code == errorCode)
        {
            closeWithoutReceiving();
            error("connect");
        }
    }
    class ConnectionClosedException : public std::exception
    {
        virtual const char* what() const noexcept override
        {
            return "Connection closed";
        }
    };

    Coroutine<void> send(const std::string &buff)
    {
        size_t length = 0;
        size_t totalLength = 0;
        DEBUG << "sending" << buff.size() << "bytes of data starting with" << buff.substr(0, std::min(buff.size(), 16UL)) << "... on socket" << socket;
        while (totalLength != buff.size())
        {
            do
            {
                code = length = ::send(socket, buff.c_str() + totalLength, buff.size() - totalLength, MSG_DONTWAIT | MSG_NOSIGNAL);
                if (code != errorCode)
                    totalLength += length;
                DEBUG << "send on socket" << socket << "returned" << code << "errno is" << errno;
                if (tryAgain(code))
                    co_await std::suspend_always();
                else
                    break;
            } while (true);

            if (code == errorCode)
                throw ConnectionClosedException();

            DEBUG << length << "bytes sent";
            DEBUG << buff.size() - totalLength << "bytes left";
        }
        DEBUG << "all bytes sent on socket" << socket;
        co_return;
    }

    Coroutine<std::string> receive()
    {
        size_t length = 0;
        std::string buff(8192, 0);
        DEBUG << "receiving on socket" << socket;
        do
        {
            code = length = recv(socket, buff.data(), buff.size(), MSG_DONTWAIT);
            DEBUG << "recv on socket" << socket << "returned" << code << "errno is" << errno;
            if (tryAgain(code))
                co_await std::suspend_always();
            else
                break;
        } while (true);

        if (code != errorCode)
        {
            DEBUG << "received" << length << "bytes of data starting with" << buff.substr(0, std::min(buff.size(), 16UL)) << "... on socket" << socket;
            buff.resize(length);
        }
        else
                throw ConnectionClosedException();

        if (buff.empty())
            throw ConnectionClosedException();

        co_return buff;
    }

    void close()
    {
        DEBUG << "start close socket" << socket;
        code = shutdown(socket, SHUT_WR);
        if (code < 0)
        {
            closeWithoutReceiving();
            return;
        }
        std::string buff(4096, 0);
        do
        {
            code = recv(socket, buff.data(), buff.size(), 0);
        } while (code > 0);
        closeWithoutReceiving();
    }

    ~TcpClient()
    {
        if (socket != invalidSocket)
            close();
    }

private:
    void closeWithoutReceiving()
    {
        DEBUG << "close socket" << socket;
        ::close(socket);
        socket = invalidSocket;
    }

    TcpClient(Socket clientSocket) { socket = clientSocket; }
    int code;
    friend class TcpServer;
};

#endif /* TcpClient_H */
