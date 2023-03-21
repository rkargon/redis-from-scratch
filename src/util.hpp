#pragma once

#include <charconv>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <type_traits>

constexpr std::size_t K_MAX_MSG = 4096;
constexpr std::size_t K_BUFF_SIZE = 4 + K_MAX_MSG + 1;

template <typename T,
          typename = typename std::enable_if_t<std::is_arithmetic_v<T>, T>>
T str_to_int_safe(const char *first, const char *last) {
  int x;
  const char *end_ptr;
  T output;
  std::from_chars_result res = std::from_chars(first, last, output, 10);
  if (res.ec != std::errc{} || res.ptr == first) {
    throw std::invalid_argument("Parsing error");
  }
  return output;
}

void die(const char *msg);

typedef int file_descriptor_t;
typedef std::int32_t errno_t;

errno_t read_full(file_descriptor_t fd, char *buf, std::size_t n);
errno_t write_all(file_descriptor_t fd, char *buf, std::size_t n);

errno_t read_request(file_descriptor_t connfd, char *rbuf,
                     std::size_t buff_size);
errno_t write_request(file_descriptor_t connfd, const char *s, std::size_t len);
errno_t write_request(file_descriptor_t connfd, std::string s);
