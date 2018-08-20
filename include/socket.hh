#ifndef INCLUDE_SOCKET_HH_
#define INCLUDE_SOCKET_HH_

#include <string>
#include <memory>

class Socket {
 public:
    virtual ~Socket() = default;
    virtual char getc() = 0;
    virtual ssize_t read(char* buf, size_t buf_len) = 0;
    virtual std::string readline();
    virtual void write(std::string const&) = 0;
    virtual void write(char const * const buf, size_t buf_len) = 0;
};

typedef std::unique_ptr<Socket> Socket_t;

class SocketAcceptor {
 public:
    virtual Socket_t accept_connection() const = 0;
};

#endif  // INCLUDE_SOCKET_HH_
