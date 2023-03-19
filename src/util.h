# pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <cstdint>
#include <cstdio>

constexpr std::size_t K_MAX_MSG = 4096;
constexpr std::size_t K_BUFF_SIZE = 4 + K_MAX_MSG + 1;

void die(const char *msg);

typedef int file_descriptor_t;
typedef std::int32_t errno_t;

errno_t read_full(file_descriptor_t fd, char *buf, std::size_t n);
errno_t write_all(file_descriptor_t fd, char *buf, std::size_t n);

errno_t read_request(file_descriptor_t connfd, char *rbuf, std::size_t buff_size);
errno_t write_request(file_descriptor_t connfd, const char *s, std::size_t len);
errno_t write_request(file_descriptor_t connfd, std::string s);
