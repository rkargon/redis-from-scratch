#include "server.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "util.hpp"

void fd_set_nb(file_descriptor_t fd) {
  int flags = die_on_errno([fd]() { return fcntl(fd, F_GETFL, 0); },
                           "fcntl get flags error");
  flags |= O_NONBLOCK;
  die_on_errno([fd, flags]() { fcntl(fd, F_SETFL, flags); },
               "fcntl set flags error");
}

void run_server() {
  std::cout << "Running server..." << std::endl;
  file_descriptor_t fd = socket(AF_INET, SOCK_STREAM, 0);
  int val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(0);  // wildcard address 0.0.0.0

  int rv = bind(fd, (const sockaddr*)&addr, sizeof(addr));
  if (rv) {
    die("bind()");
  }

  rv = listen(fd, SOMAXCONN);
  if (rv) {
    die("listen()");
  }

#if 0
  while (true) {
    // accept
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    int connfd = accept(fd, (struct sockaddr*)&client_addr, &socklen);
    if (connfd < 0) {
      continue;  // error
    }

    while (true) {
      std::int32_t err = handle_client_single_request(connfd);
      if (err) {
        break;
      }
    }

    close(connfd);
  }
#endif

  std::unordered_map<file_descriptor_t, Connection> conn_by_fd;
  fd_set_nb(fd);

  std::vector<pollfd> poll_args;
  while (true) {
    poll_args.clear();
    // put listening fd first
    poll_args.push_back({fd, POLLIN, 0});
    for (const auto& [_, conn] : conn_by_fd) {
      short events = (conn.state == conn_state::STATE_REQ) ? POLLIN : POLLOUT;
      events |= POLLERR;
      poll_args.push_back({conn.fd, events});
    }

    // poll for active fds
    // the timeout argument doesn't matter here
    int rv = poll(poll_args.data(), (nfds_t)poll_args.size(), 1000);
    if (rv < 0) {
      die("poll");
    }

    // process active connections
    for (std::size_t i = 1; i < poll_args.size(); ++i) {
      const pollfd& poll_arg = poll_args[i];
      if (poll_arg.revents) {
        Connection& conn = conn_by_fd.at(poll_arg.fd);
        connection_io(conn);  // TODO
        if (conn.state == conn_state::STATE_END) {
          close(conn.fd);
          conn_by_fd.erase(conn.fd);
        }
      }
    }

    // try to accept a new connection if the listening fd is active
    if (poll_args[0].revents) {
      accept_new_conn(conn_by_fd, fd);  // TODO
    }
  }
}

errno_t accept_new_conn(
    std::unordered_map<file_descriptor_t, Connection>& conn_by_fd,
    file_descriptor_t fd) {
  // accept
  struct sockaddr_in client_addr = {};
  socklen_t socklen = sizeof(client_addr);
  file_descriptor_t connfd =
      accept(fd, (struct sockaddr*)&client_addr, &socklen);
  if (connfd < 0) {
    std::cerr << "accept() error" << std::endl;
    return -1;  // error
  }

  // set the new connection fd to nonblocking mode
  fd_set_nb(connfd);

  conn_by_fd.emplace(connfd, connfd);
  return 0;
}

std::int32_t handle_client_single_request(const file_descriptor_t connfd) {
  char rbuf[K_BUFF_SIZE];
  if (errno_t err = read_request(connfd, rbuf, K_BUFF_SIZE)) {
    std::cerr << "read() error." << std::endl;
    return err;
  }

  // do something
  std::cout << "RECV: " << &rbuf[4] << std::endl;

  // reply using the same protocol
  std::cout << "SEND: "
            << "world" << std::endl;
  return write_request(connfd, "world");
}

void connection_io(Connection& conn) {
  if (conn.state == STATE_REQ) {
    state_req(conn);
  } else if (conn.state == STATE_RES) {
    state_res(conn);
  } else {
    assert(0);  // not expected
  }
}

void state_req(Connection& conn) {
  while (try_fill_buffer(conn)) {
  }
}

bool try_fill_buffer(Connection& conn) {
  // try to fill the buffer
  assert(conn.rbuf_size < Connection::BUFFER_SIZE);
  ssize_t rv = 0;
  do {
    std::size_t cap = Connection::BUFFER_SIZE - conn.rbuf_size;
    rv = read(conn.fd, &conn.rbuf[conn.rbuf_size], cap);
  } while (rv < 0 && errno == EINTR);
  if (rv < 0 && errno == EAGAIN) {
    // got EAGAIN, stop.
    return false;
  }
  if (rv < 0) {
    std::cerr << "read() error" << std::endl;
    conn.state = STATE_END;
    return false;
  }
  if (rv == 0) {
    if (conn.rbuf_size > 0) {
      std::cerr << "unexpected EOF" << std::endl;
    } else {
      std::cerr << "EOF" << std::endl;
    }
    conn.state = STATE_END;
    return false;
  }

  conn.rbuf_size += (std::size_t)rv;
  assert(conn.rbuf_size <= Connection::BUFFER_SIZE - conn.rbuf_size);

  // Try to process requests one by one.
  // Why is there a loop? Please read the explanation of "pipelining".
  while (try_one_request(conn)) {
  }
  return (conn.state == STATE_REQ);
}

bool try_one_request(Connection& conn) {
  // try to parse a request from the buffer
  if (conn.rbuf_size < 4) {
    // not enough data in the buffer. Will retry in the next iteration
    return false;
  }

  std::uint32_t len = *reinterpret_cast<std::uint32_t*>(conn.rbuf.get());
  if (len > K_MAX_MSG) {
    std::cerr << "too long!" << std::endl;
    conn.state = conn_state::STATE_END;
    return false;
  }
  if (4 + len > conn.rbuf_size) {
    // not enough data in the buffer. Will retry in the next iteration
    return false;
  }
  std::cout << "Client says: ";
  std::cout.write(&conn.rbuf[4], len);
  std::cout << std::endl;

  // Create echoing response
  std::memcpy(conn.wbuf.get(), conn.rbuf.get(), len + 4);
  conn.wbuf_size = 4 + len;

  // Remove request data from read buffer
  std::size_t remain = conn.rbuf_size - 4 - len;
  if (remain) {
    std::memmove(conn.rbuf.get(), &conn.rbuf[4 + len], remain);
  }
  conn.rbuf_size = remain;

  // change state
  conn.state = conn_state::STATE_RES;
  state_res(conn);

  // continue the outer loop if the request was fully processed
  return (conn.state == conn_state::STATE_REQ);
}

void state_res(Connection& conn) {
  while (try_flush_buffer(conn)) {
  }
}

bool try_flush_buffer(Connection& conn) {
  ssize_t rv = 0;
  do {
    std::size_t remain = conn.wbuf_size - conn.wbuf_sent;
    rv = write(conn.fd, &conn.wbuf[conn.wbuf_sent], remain);
  } while (rv < 0 && errno == EINTR);
  if (rv < 0 && errno == EAGAIN) {
    // got EAGAIN, stop.
    return false;
  }
  if (rv < 0) {
    std::cerr << "write() error" << std::endl;
    conn.state = conn_state::STATE_END;
    return false;
  }
  conn.wbuf_sent += (std::size_t)rv;
  assert(conn.wbuf_sent <= conn.wbuf_size);
  if (conn.wbuf_sent == conn.wbuf_size) {
    // response was fully sent, change state back
    conn.state = conn_state::STATE_REQ;
    conn.wbuf_sent = 0;
    conn.wbuf_size = 0;
    return false;
  }
  // still got some data in wbuf, could try to write again
  return true;
}

int main() {
  run_server();
  return 0;
}
