# Redis-From-Scratch

Toy redis implementation based off of [https://build-your-own.org/redis/](https://build-your-own.org/redis/).

## Quickstart

```sh
git clone git@github.com:rkargon/redis-from-scratch.git
cd redis-from-scratch/

# build
make all

# run
./build/server
./build/client
```

## TODO

Chapter 4:
* Buffered IO for socket read/write calls (perhaps some sort of streambuf?)

Chapter 6:
* Try to use `epoll` instead of `poll` in the event loop. This should be easy.
* We are using `memmove` to reclaim read buffer space. However, memmove on every request is unnecessary, change the code the perform memmove only before read.
* In the `state_res` function, write was performed for a single response. In pipelined sceneries, we could buffer multiple responses and flush them in the end with a single write call. Note that the write buffer could be full in the middle.

Misc:
* Create object-oriented wrapper for C sockets
* CLI args for port number/other init options
* Use protocol buffers for serde
* Use boost:asio