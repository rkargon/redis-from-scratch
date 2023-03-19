#include "util.h"

#include <cstdlib>
#include <iostream>
#include <memory>

#include "unistd.h"

void die(const char *msg) {
  std::cerr << msg << std::endl;
  std::exit(1);
}

errno_t read_full(file_descriptor_t fd, char *buf, std::size_t n) {
  while (n > 0) {
    std::ptrdiff_t rv = read(fd, buf, n);
    if (rv <= 0) {
      return -1;
    }
    assert((std::size_t)rv <= n);
    n -= rv;
    buf += rv;
  }
  return 0;
}

errno_t write_all(file_descriptor_t fd, char *buf, std::size_t n) {
  while (n > 0) {
    std::ptrdiff_t rv = write(fd, buf, n);
    if (rv <= 0) {
      return -1;  // error
    }
    assert((std::size_t)rv <= n);
    n -= rv;
    buf += rv;
  }
  return 0;
}

errno_t read_request(file_descriptor_t connfd, char *rbuf,
                     std::size_t buff_size) {
  std::size_t max_message_len = buff_size - 5;
  // read 4 bytes header
  errno = 0;
  errno_t err = read_full(connfd, rbuf, 4);
  if (err) {
    std::cerr << (errno ? "read() error" : "EOF") << std::endl;
    return err;
  }

  std::size_t len = 0;
  std::memcpy(&len, rbuf, 4);  // assume little endian
  if (len > max_message_len) {
    std::cerr << "too long: " << len << std::endl;
    return -1;
  }

  // read msg body
  err = read_full(connfd, &rbuf[4], len);
  if (err) {
    std::cerr << "read() error." << std::endl;
    return err;
  }
  rbuf[4 + len] = '\0';
  return 0;
}

errno_t write_request(file_descriptor_t connfd, const char *s,
                      std::size_t len) {
  std::size_t buff_size = 4 + len;
  std::unique_ptr<char[]> buff(new char[buff_size]);
  std::memcpy(buff.get(), &len, 4);
  std::memcpy(&buff[4], s, len);
  return write_all(connfd, buff.get(), buff_size);
}

errno_t write_request(file_descriptor_t connfd, std::string s) {
  return write_request(connfd, s.data(), s.size());
}