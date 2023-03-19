#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

// typedef struct sockaddr_in sockaddr_in_t;

void die(const char *msg) {
  std::cerr << msg << std::endl;
  std::exit(1);
}

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

void run_client() {
  std::cout << "Running client..." << std::endl;
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    die("socket()");
  }
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);  // 127.0.0.1
  int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("connect");
  }

  char msg[] = "hello";
  write(fd, msg, strlen(msg));

  char rbuf[64] = {};
  ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
  if (n < 0) {
    die("read");
  }
  std::cout << "Server sez: " << rbuf << std::endl;
  close(fd);
}

int main(int argc, char **argv) {
  if (argc == 2) {
    if (argc == 2 && std::strcmp(argv[1], "server") == 0) {
      run_server();
    } else if (argc == 2 && std::strcmp(argv[1], "client") == 0) {
      run_client();
    }
  }
  return 0;
}
