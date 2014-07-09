/*
 * Copyright (C) 2014  Mozilla Foundation
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

#include <assert.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fdio/loop.h>
#include "log.h"

#define MAXNFDS 64

#define ARRAYLEN(x) \
  (sizeof(x) / sizeof(x[0]))

struct fd_state {
  struct epoll_event event;
  enum ioresult (*func)(int, uint32_t, void*);
  void* data;
};

static struct fd_state fd_state[MAXNFDS];
static int epfd;

static int
fd_is_valid(int fd)
{
  if (fd < 0) {
    ALOGE("invalid file descriptor %d", fd);
    return 0;
  } else if (fd >= (ssize_t)ARRAYLEN(fd_state)) {
    ALOGE("file descriptors %d out of range", fd);
    return 0;
  }
  return 1;
}

int
add_fd_to_epoll_loop(int fd, uint32_t epoll_events,
                     enum ioresult (*func)(int, uint32_t, void*), void* data)
{
  int enabled;
  int res;

  if (!fd_is_valid(fd))
    return -1;
  if (!func) {
    ALOGE("no callback function specified");
    return -1;
  }

  enabled = !!fd_state[fd].event.events;

  fd_state[fd].event.events = epoll_events;
  fd_state[fd].event.data.fd = fd;

  if (enabled)
    res = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &fd_state[fd].event);
  else
    res = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &fd_state[fd].event);

  if (res < 0) {
    ALOGE_ERRNO("epoll_ctl");
    return -1;
  }

  fd_state[fd].func = func;
  fd_state[fd].data = data;

  return 0;
}

/* returns bits at position 'i' or above */
static int
bits_above(uint32_t epoll_events, unsigned int i)
{
  assert(i < 32);
  return epoll_events & ~((1<<i)-1);
}

static enum ioresult
fd_events_func(int fd, uint32_t epoll_events, const struct fd_events* evfuncs)
{
  enum ioresult res;
  uint32_t bit;

  assert(evfuncs);

  for (bit = 0, res = IO_OK;
       bits_above(epoll_events, bit) && res == IO_OK;
     ++bit) {
    enum ioresult (*func)(int, void*);
    switch (epoll_events & (1ul<<bit)) {
      case 0:
        /* no event at current bit */
        continue;
      case EPOLLIN:
        func = evfuncs->epollin;
        break;
      case EPOLLPRI:
        func = evfuncs->epollpri;
        break;
      case EPOLLOUT:
        func = evfuncs->epollout;
        break;
      case EPOLLERR:
        func = evfuncs->epollerr;
        break;
      case EPOLLHUP:
        func = evfuncs->epollhup;
        break;
      default:
        ALOGE("unknown event bit 0x%x", bit);
        continue;
    }
    if (func) {
      res = func(fd, evfuncs->data);
    } else {
      ALOGE("unhandled event bit 0x%x", bit);
    }
  }
  return res;
}

static enum ioresult
fd_events_func_cb(int fd, uint32_t epoll_events, void* data)
{
  return fd_events_func(fd, epoll_events, data);
}

int
add_fd_events_to_epoll_loop(int fd, uint32_t epoll_events,
                            const struct fd_events* evfuncs)
{
  if (!evfuncs) {
    ALOGE("no callback structure specified");
    return -1;
  }
  return add_fd_to_epoll_loop(fd, epoll_events, fd_events_func_cb,
                              (void*)evfuncs);
}

static void
clear_fd_state(struct fd_state *fd_state)
{
  assert(fd_state);

  fd_state->event.events = 0;
  /* use the largest union member to clean allocated memory*/
  fd_state->event.data.u64 = 0;
  fd_state->func = NULL;
  fd_state->data = NULL;
}


void
remove_fd_from_epoll_loop(int fd)
{
  int res;

  if (!fd_is_valid(fd))
    return;

  assert(fd_state[fd].events);

  res = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
  if (res < 0)
    ALOGW_ERRNO("epoll_ctl");

  clear_fd_state(&fd_state[fd]);
}

static enum ioresult
epoll_loop_iteration(void)
{
  struct epoll_event events[MAXNFDS];
  int nevents, i;
  enum ioresult res;

  nevents = TEMP_FAILURE_RETRY(epoll_wait(epfd, events, ARRAYLEN(events), -1));
  if (nevents < 0) {
    ALOGE_ERRNO("epoll_wait");
    return IO_ABORT;
  }

  for (i = 0, res = IO_OK; i < nevents && res == IO_OK; ++i) {
    int fd = events[i].data.fd;

    assert(fd_state[fd].func);
    res = fd_state[fd].func(fd, events[i].events, fd_state[fd].data);
  }
  return res;
}

int
epoll_loop(enum ioresult (*init)(void*), void* data)
{
  enum ioresult res;

  epfd = epoll_create(ARRAYLEN(fd_state));
  if (epfd < 0) {
    ALOGE_ERRNO("epoll_create");
    return -1;
  }

  if (init && (res = init(data)) == IO_ABORT)
    goto err_init;
  else
    res = IO_OK;

  while (res == IO_OK || res == IO_POLL) {
    res = epoll_loop_iteration();
  }

  if (res == IO_ABORT)
    goto err_epoll_loop_iteration;

  if (TEMP_FAILURE_RETRY(close(epfd)) < 0)
    ALOGW_ERRNO("close");

  return 0;
err_epoll_loop_iteration:
err_init:
  if (TEMP_FAILURE_RETRY(close(epfd)) < 0)
    ALOGW_ERRNO("close");
  return -1;
}
