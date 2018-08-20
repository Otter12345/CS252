#ifndef INCLUDE_TLS_HH_
#define INCLUDE_TLS_HH_

#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <sys/socket.h>

#include <string>

#include "socket.hh"

class TLSSocket : public Socket {
 private:
    const int _socket;
    const struct sockaddr_in _addr;
    SSL* _ssl;
 public:
    TLSSocket(int port_no, struct sockaddr_in _addr, SSL* ssl);
    ~TLSSocket() noexcept;
    char getc();
    ssize_t read(char* buf, size_t buf_len);
    std::string readline();
    void write(std::string const&);
    void write(char const * const buf, size_t len);
};

class TLSSocketAcceptor : public SocketAcceptor {
 private:
    int _master_socket;
    struct sockaddr_in _addr;
    SSL_CTX* _ssl_ctx;
 public:
    explicit TLSSocketAcceptor(const int portno);
    ~TLSSocketAcceptor() noexcept;

    Socket_t accept_connection() const;
};

#endif  // INCLUDE_TLS_HH_
