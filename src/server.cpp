#include "server.hpp"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "util.hpp"

void run_server() {
  std::cout << "Running server..." << std::endl;
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  int val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(0);  // wildcard address 0.0.0.0

  int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("bind()");
  }

  rv = listen(fd, SOMAXCONN);
  if (rv) {
    die("listen()");
  }

  while (true) {
    // accept
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
    if (connfd < 0) {
      continue;  // error
    }

    while (true) {
      std::int32_t err = handle_client_single_request(connfd);
      if (err) {
        break;
      }
    }

    close(connfd);
  }
}

std::int32_t handle_client_single_request(const file_descriptor_t connfd) {
  char rbuf[K_BUFF_SIZE];
  if (errno_t err = read_request(connfd, rbuf, K_BUFF_SIZE)) {
    std::cerr << "read() error." << std::endl;
    return err;
  }

  // do something
  std::cout << "RECV: " << &rbuf[4] << std::endl;

  // reply using the same protocol
  std::cout << "SEND: " << "world" << std::endl;
  return write_request(connfd, "world");
}

int main() {
  run_server();
  return 0;
}
