#include "server.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "server.h"
#include "util.h"

void handle_client_conn(const int connfd) {
  char rbuf[64] = {};
  ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
  if (n < 0) {
    std::cerr << "read() error" << std::endl;
    return;
  }
  std::cout << "Client says: " << rbuf << std::endl;
  char wbuf[] = "world";
  write(connfd, wbuf, std::strlen(wbuf));
}

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
    handle_client_conn(connfd);
    close(connfd);
  }
}

int main(int argc, char **argv) {
  run_server();
  return 0;
}
