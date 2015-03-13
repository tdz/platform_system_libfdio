/*
 * Copyright (C) 2014-2015  Mozilla Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdint.h>
#include "ioresult.h"

/*
 * This file provides the API to the epoll main loop.
 *
 * You can start the main loop by calling |epoll_loop|. It will call the
 * supplied init function with the argument |data| before starts polling.
 *
 * The init function shall create an initial set of file descriptors and
 * add them to the poll loop, using |add_fd_to_epoll_loop|. Events are
 * selected using the second argument; see 'man 2 epoll_ctl' for a list
 * of available events. If an event is received on a file descriptor, the
 * epoll loop calls the supplied function |func| with the file descriptor,
 * the event flag and the user data as arguments.
 *
 * For convenience, |add_fd_events_to_epoll_loop| allows to map events
 * to callback functions automatically. It receives a structure with a
 * callback for each event and will call the correct function when the
 * event happens.
 *
 * File descriptors can be removed from the poll loop using
 * |remove_fd_from_epoll_loop|.
 *
 * The interface is _not_ thread-safe.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct fd_events {
  void* data;
  enum ioresult (*epollin)(int, void*);
  enum ioresult (*epollpri)(int, void*);
  enum ioresult (*epollout)(int, void*);
  enum ioresult (*epollerr)(int, void*);
  enum ioresult (*epollhup)(int, void*);
};

enum ioresult
fd_events_handler(int fd, uint32_t epoll_events, void* data);

int
add_fd_to_epoll_loop(int fd, uint32_t epoll_events,
                     enum ioresult (*func)(int, uint32_t, void*), void* data);

int
add_fd_events_to_epoll_loop(int fd, uint32_t epoll_events,
                            const struct fd_events* evfuncs);

void
remove_fd_from_epoll_loop(int fd);

int
epoll_loop(enum ioresult (*init)(void*), void (*uninit)(void*), void* data);

#ifdef __cplusplus
}
#endif
