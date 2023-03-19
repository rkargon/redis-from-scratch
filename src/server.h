# pragma once

#include <cstdint>
#include <cstdio>

void run_server();
std::int32_t handle_client_single_request(const int connfd);
