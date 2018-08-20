#ifndef INCLUDE_TCP_HH_
#define INCLUDE_TCP_HH_

#include <arpa/inet.h>
#include <sys/socket.h>

#include <string>

#include "socket.hh"

class TCPSocket : public Socket {
 private:
    const int _socket;
    const struct sockaddr_in _addr;
 public:
    TCPSocket(int port_no, struct sockaddr_in _addr);
    ~TCPSocket() noexcept;
    char getc();
    ssize_t read(char* buf, size_t buf_len);
    std::string readline();
    void write(std::string const&);
    void write(char const * const buf, size_t len);
};

class TCPSocketAcceptor : public SocketAcceptor {
 private:
    int _master_socket;
    struct sockaddr_in addr;
 public:
    explicit TCPSocketAcceptor(const int portno);
    ~TCPSocketAcceptor() noexcept;

    Socket_t accept_connection() const;
};

#endif  // INCLUDE_TCP_HH_
