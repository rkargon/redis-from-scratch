#include <sys/socket.h>

#include <cstring>
#include <iostream>

void run_server() {}

void run_client() {}

int main(int argc, char **argv) {
  if (argc == 2 && std::strcmp(argv[1], "server") == 0) {
    std::cout << "server" << std::endl;
  } else if (argc == 2 && std::strcmp(argv[1], "client") == 0) {
    std::cout << "client" << std::endl;
  }
  return 0;
}
