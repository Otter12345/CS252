/**
 * This file contains the implementation for Socket::readline()
 * You should not need to edit this file unless you add methods to the Socket or SocketAdaptor classes
 *
 * See include/socket.hh for more details
 */

#include "socket.hh"
#include <iostream>

std::string Socket::readline() {
    std::string str;
    char c;
    while (true) {
        c = getc();
        if (c == EOF) {
            break;
        }
        str += c;
        if (str.length() >= 2 && (0 == str.compare(str.length() - 2, 2, "\r\n"))) {
            break;
        }
    }

    return str;
}
