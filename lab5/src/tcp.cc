/**
 * This file provides an implementation of a Socket and SocketAdaptor using basic
 * TCP. You will want to understand what is going on in this file for Task 2.1, but 
 * no modifications to this file should be necessary.
 */

#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <fstream>
#include "errors.hh"
#include "tcp.hh"
#include <chrono>
#include <ctime>
#include <mutex>

extern std::mutex mtx;
TCPSocket::TCPSocket(int port_no, struct sockaddr_in addr) : _socket(port_no), _addr(addr) {
	mtx.lock();
	char inet_pres[INET_ADDRSTRLEN];
    // sin_family will be AF_INET
    if (inet_ntop(addr.sin_family, &(addr.sin_addr), inet_pres, INET_ADDRSTRLEN)) {
        std::cout << "Received a connection from " << inet_pres << std::endl;
    }
	extern std::chrono::system_clock::time_point start;
	start = std::chrono::system_clock::now();
	extern std::vector<std::string> IPaddress;
	IPaddress.push_back(std::string(inet_pres));
	mtx.unlock();
}

TCPSocket::~TCPSocket() noexcept {
    std::cout << "Closing TCP socket fd " << _socket;
    char inet_pres[INET_ADDRSTRLEN];
    // sin_family will be AF_INET
    if (inet_ntop(_addr.sin_family, &(_addr.sin_addr), inet_pres, INET_ADDRSTRLEN)) {
        std::cout << " from " << inet_pres;
    }
    std::cout << std::endl;
    close(_socket);
   extern std::vector<std::string> IPaddress;
   extern std::vector<std::string> respCode;
   extern std::vector<std::string> routes;
   //IPaddress.clear();
   routes.clear();
   respCode.clear();
}

char TCPSocket::getc() {
    char c;
    ssize_t read = recv(_socket, &c, 1, 0);
    if (read < 0) {
        throw ConnectionError("Unable to read a character: " + std::string(strerror(errno)));
    } else if (read > 1) {
        throw ConnectionError("Read more than one byte when expecting to only read one.");
    } else if (read == 0) {
        c = EOF;
    }
    return c;
}

ssize_t TCPSocket::read(char* buf, size_t buf_len) {
    ssize_t r = recv(_socket, buf, buf_len, 0);
    if (r == -1) {
        throw ConnectionError("Unable to read a character: " + std::string(strerror(errno)));
    }
    return r;
}

std::string TCPSocket::readline() {
    std::string str;
    char c;
    while ((c = getc()) != '\n' && c != EOF) {
        str.append(1, c);
    }
    if (c == '\n') {
        str.append(1, '\n');
    }
    return str;
}

void TCPSocket::write(std::string const& str) {
    write(str.c_str(), str.length());
}

void TCPSocket::write(char const * const buf, const size_t buf_len) {
    if (buf == NULL) return;
  int ret_code = send(_socket, buf, buf_len, 0);
  if (ret_code == -1) {
      throw ConnectionError("Unable to write: " + std::string(strerror(errno)));
  } else if ((size_t)ret_code != buf_len) {
      size_t i;
      std::stringstream buf_hex_stream;
      for (i = 0; i < buf_len; i++)
        buf_hex_stream << std::hex << buf[i];

      throw ConnectionError("Could not write all bytes of: \'" + buf_hex_stream.str() +
          "\'. Expected " + std::to_string(buf_len) + " but actually sent " +
          std::to_string(ret_code));
  }
}

TCPSocketAcceptor::TCPSocketAcceptor(const int portno) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(portno);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    _master_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_master_socket < 0) {
        throw ConnectionError("Unable to create socket: " + std::string(strerror(errno)));
    }

    int optval = 1;
    if (setsockopt(_master_socket, SOL_SOCKET,  SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        throw ConnectionError("Unable to set socket options: " + std::string(strerror(errno)));
    }
    if (bind(_master_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        throw ConnectionError("Unable to bind to socket: " + std::string(strerror(errno)));
    }

    if (listen(_master_socket, 50) < 0) {
        throw ConnectionError("Unable to listen to socket: " + std::string(strerror(errno)));
    }
}

Socket_t TCPSocketAcceptor::accept_connection() const {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int s = accept(_master_socket, (struct sockaddr*)&addr, &addr_len);
    if (s == -1) {
        throw ConnectionError("Unable to accept connection: " + std::string(strerror(errno)));
    }
    return std::make_unique<TCPSocket>(s, addr);
}

TCPSocketAcceptor::~TCPSocketAcceptor() noexcept {
    std::cout << "Closing socket " << _master_socket << std::endl;
    close(_master_socket);
}
