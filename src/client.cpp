#include "client.hpp"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "server.hpp"
#include "util.hpp"

void run_client() {
  std::cout << "Running client..." << std::endl;
  file_descriptor_t fd = socket(AF_INET, SOCK_STREAM, 0);
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

  for (int i = 0; i < 3; ++i) {
    if (errno_t err = query(fd, "ouch!")) {
      break;
    }
  }

  close(fd);
}

errno_t query(file_descriptor_t fd, std::string text) {
  std::cout << "SEND: " << text << std::endl;

  if (errno_t err = write_request(fd, text)) {
    std::cerr << "write() error" << std::endl;
    return err;
  }

  char rbuf[K_BUFF_SIZE];
  if (errno_t err = read_request(fd, rbuf, K_BUFF_SIZE)) {
    return err;
  }
  std::cout << "RECV: " << (rbuf + 4) << std::endl;
  return 0;
}

int main(void) {
  run_client();
  return 0;
}
