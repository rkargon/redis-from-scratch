#include "sprockets.hpp"

#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h> 
#include <regex>
#include <unistd.h>

namespace sprockets {

// "a.b.c.d:port"
struct sockaddr_in operator"" ipv4(const char* s, std::size_t len) {
  //  TODO
  const char* port_start = std::strchr(s, ':');
  std::atoi(port_start);
  std::regex re(R"(\d+)\.(\d+)\.(\d+)\.(\d+):(\d+)");

  std::cmatch match_result;
  if (std::regex_match(s, match_result, re)){
    // match_result.
  }
}
 
struct sockaddr_in make_ipv4_address(std::string addr, int port) {
  return make_ipv4_address(addr.data(), addr.size(), port);
}
struct sockaddr_in make_ipv4_address(const char* s, std::size_t len, int port) {
  long val = 0;

  int octet_start = 0;
  int octet_offset = 24;
  const char* start = s;
  const char* end = nullptr;
  int i;
  for (i = 1; i <= 4; ++i) {
    if (std::distance(s, start) >= len) {
      break;
    }
    long octet = std::strtol(start, (char**)&end, 10);
    val |= (octet << octet_offset);
    start = end;
    octet_offset -= 8;
  }

  if (i <= 4 || start != (s + len)) {
    throw std::invalid_argument("Bad IP address");
  }

  struct sockaddr_in addr_struct = {};
  addr_struct.sin_family = AF_INET;
  addr_struct.sin_port = ntohs(port);
  addr_struct.sin_addr.s_addr = ntohl(val);  // wildcard address 0.0.0.0
  return addr_struct;
}

Socket::~Socket() { close(this->_fd); }
template <typename T>
errno_t Socket::bind(const T& addr) {
  return ::bind(_fd, (const sockaddr*)&addr, sizeof(addr));
}

}  // namespace sprockets