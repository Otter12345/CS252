#ifndef INCLUDE_ERRORS_HH_
#define INCLUDE_ERRORS_HH_

#include <stdexcept>
#include <string>

class ConnectionError : public std::runtime_error {
 public:
    explicit ConnectionError(std::string const& what) : std::runtime_error(what) { }
    explicit ConnectionError(char const* what) : std::runtime_error(what) { }
};

class ParseError : public std::runtime_error {
 public:
    explicit ParseError(std::string const& what) : std::runtime_error(what) { }
    explicit ParseError(char const* what) : std::runtime_error(what) { }
};

#endif  // INCLUDE_ERRORS_HH_
