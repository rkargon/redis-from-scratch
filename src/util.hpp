#pragma once

#include <charconv>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

typedef int file_descriptor_t;
typedef std::int32_t errno_t;

constexpr std::size_t K_MAX_MSG = 4096;
constexpr std::size_t K_BUFF_SIZE = 4 + K_MAX_MSG + 1;

template <typename T,
          typename = typename std::enable_if_t<std::is_arithmetic_v<T>, T>>
T str_to_int_safe(const char *first, const char *last) {
  T output;
  std::from_chars_result res = std::from_chars(first, last, output, 10);
  if (res.ec != std::errc{} || res.ptr == first) {
    throw std::invalid_argument("Parsing error");
  }
  return output;
}

template <typename Iterable>
void print_iterable(const Iterable &iterable, std::ostream &os = std::cout) {
  os << "[";
  for (const auto &i : iterable) {
    os << i << ", ";
  }
  os << "]" << std::endl;
}

template <typename Iterable, typename UnaryFunction>
void print_transform_iterable(const Iterable &iterable, UnaryFunction f,
                              std::ostream &os = std::cout) {
  os << "[";
  for (const auto &i : iterable) {
    os << f(i) << ", ";
  }
  os << "]" << std::endl;
  ;
}

// template <typename F>
// class wrap_errno {
//   using R = std::invoke_result_t<F>;
//   using return_type =
//       std::conditional_t<std::is_void_v<R>, std::optional<errno_t>,
//                          std::variant<R, errno_t>>;

//   wrap_errno(const F &f) : _f(f) {}
//   wrap_errno(const F &&f) : _f(std::forward<>(f)) {}

//   return_type operator()() {
//     errno = 0;
//     R result = f();
//     errno_t err = errno;
//     errno = 0;
//     if (err) {
//       return std::variant<R, errno_t>(std::in_place_index<1>, err);
//     } else {
//       return std::variant<R, errno_t>(std::in_place_index<0>, result);
//     }
//   }

//   const F &_f;
// };

void die(const char *msg);

// TODO void
template <typename Callable,
          typename =
              std::enable_if_t<!std::is_void_v<std::invoke_result_t<Callable>>>>
std::invoke_result_t<Callable> die_on_errno(Callable f, const char *msg) {
  errno = 0;
  std::invoke_result_t<Callable> res = f();
  if (errno) {
    die(msg);
  }
  return res;
}

template <
    typename Callable,
    typename = std::enable_if_t<std::is_void_v<std::invoke_result_t<Callable>>>>
void die_on_errno(Callable f, const char *msg) {
  errno = 0;
  f();
  if (errno) {
    die(msg);
  }
}

// TODO RemovableIterator?

errno_t read_full(file_descriptor_t fd, char *buf, std::size_t n);
errno_t write_all(file_descriptor_t fd, char *buf, std::size_t n);

errno_t read_request(file_descriptor_t connfd, char *rbuf,
                     std::size_t buff_size);
errno_t write_request(file_descriptor_t connfd, const char *s, std::size_t len);
errno_t write_request(file_descriptor_t connfd, std::string s);
