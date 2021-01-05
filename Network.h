#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "Coroutine.h"
#include "error.h"

class Network
{
public:
    using Socket = int;
    static constexpr Socket invalidSocket = -1;

    Socket getSocket() const
    {
        return socket;
    }

protected:
    Socket socket = invalidSocket;
    bool tryAgain(int code) const
    {
        return code == invalidSocket && (errno == EAGAIN || errno == EWOULDBLOCK);
    }
};

#endif /* NETWORK_H */
