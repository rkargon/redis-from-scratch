#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "util.hpp"

typedef enum conn_state {
  STATE_REQ = 0,
  STATE_RES = 1,
  STATE_END = 2,  // mark the connection for deletion
} conn_state_t;

// TODO these are expensive! Don't pass these around
struct Connection {
  static constexpr std::size_t BUFFER_SIZE = 4 + K_MAX_MSG;

  Connection(file_descriptor_t fd)
      : fd(fd),
        rbuf(std::make_unique<char[]>(Connection::BUFFER_SIZE)),
        wbuf(std::make_unique<char[]>(Connection::BUFFER_SIZE)) {}

  file_descriptor_t fd = -1;
  conn_state_t state = conn_state::STATE_REQ;  // either STATE_REQ or STATE_RES
  // buffer for reading
  std::size_t rbuf_size = 0;
  std::unique_ptr<char[]> rbuf;
  // buffer for writing
  std::size_t wbuf_size = 0;
  std::size_t wbuf_sent = 0;
  std::unique_ptr<char[]> wbuf;
};

void fd_set_nb(int fd);
void run_server();
std::int32_t handle_client_single_request(const int connfd);

errno_t accept_new_conn(
    std::unordered_map<file_descriptor_t, Connection>& conn_by_fd,
    file_descriptor_t fd);
std::int32_t handle_client_single_request(const file_descriptor_t connfd);
void connection_io(Connection& conn);
void state_req(Connection& conn);
bool try_fill_buffer(Connection& conn);
bool try_one_request(Connection& conn);
void state_res(Connection& conn);
bool try_flush_buffer(Connection& conn);