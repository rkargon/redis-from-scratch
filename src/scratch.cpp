#include <iostream>

#include "util.hpp"

int even_numbers_only_please(int x) {
  if (x % 2) {
    errno = ENODATA;
  } else {
    errno = 0;
  }
  return x;
}

void do_something(int x) { std::cout << "do something!" << x << std::endl; }

int main(int, char**) {
  auto result =
      die_on_errno([] { return even_numbers_only_please(4); }, "even only!");
  std::cout << result << std::endl;

  die_on_errno([](){do_something(100);}, "failed to do something");

  auto result2 =
      die_on_errno([] { return even_numbers_only_please(5); }, "even only!");
  std::cout << result2 << std::endl;
}