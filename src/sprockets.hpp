#pragma once

#include <netinet/in.h>

#include <string>

#include "util.hpp"

namespace sprockets {

// "a.b.c.d:port"
struct sockaddr_in operator"" ipv4(const char* s, std::size_t len);
struct sockaddr_in make_ipv4_address(std::string addr, int port);
struct sockaddr_in make_ipv4_address(const char* s, std::size_t len, int port);
class Socket {
 public:
  ~Socket();

  template <typename T>
  errno_t bind(const T& addr);

  const int& fd() const { return _fd; }

 private:
  const int _fd;
};
}  // namespace sprockets