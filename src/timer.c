/*
 * Copyright (C) 2015  Mozilla Foundation
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
#include <ports/sys/timerfd.h>
#include <sys/epoll.h>
#include <fdio/loop.h>
#include <fdio/task.h>
#include <fdio/timer.h>
#include "log.h"

struct timer_param {
  enum ioresult (*func)(int, uint32_t, void*);
  void* data;
};

struct add_timer_param {
  int fd;
  struct timer_param param;
};

static enum ioresult
add_timer_cb(void* data)
{
  struct add_timer_param* param = data;
  assert(param);

  add_fd_to_epoll_loop(param->fd, EPOLLIN | EPOLLERR,
                       param->param.func, param->param.data);
  free(param);

  return IO_OK;
}

static enum ioresult
remove_timer_cb(void* data)
{
  int fd = (int)data;

  remove_fd_from_epoll_loop(fd);

  return IO_OK;
}

static struct timespec*
set_timespec(struct timespec* timespec,
             unsigned long long time_ms)
{
  static const unsigned long long MS_PER_S = 1000;
  static const unsigned long long NS_PER_MS = 1000000;

  timespec->tv_sec = time_ms / MS_PER_S;
  timespec->tv_nsec = (time_ms % MS_PER_S) * NS_PER_MS;

  return timespec;
}

static int
add_timer(int clockid,
          int timeout_is_absolute,
          unsigned long long timeout_ms,
          unsigned long long interval_ms,
          enum ioresult (*func)(int, uint32_t, void*),
          void* data)
{
  int fd, flags;
  struct itimerspec timeout;
  struct add_timer_param* param;

  assert(timeout_ms);

  fd = timerfd_create(clockid, TFD_NONBLOCK | TFD_CLOEXEC);
  if (fd < 0) {
    ALOGE_ERRNO("timerfd_create");
    return -1;
  }

  flags = 0;

  if (timeout_is_absolute)
    flags |= TFD_TIMER_ABSTIME;

  set_timespec(&timeout.it_value, timeout_ms);
  set_timespec(&timeout.it_interval, interval_ms);

  if (timerfd_settime(fd, flags, &timeout, NULL) < 0) {
    ALOGE_ERRNO("timerfd_settime");
    goto err_timerfd_settime;
  }

  /* install timerfd from within I/O loop */

  errno = 0;
  param = malloc(sizeof(*param));
  if (errno) {
    ALOGE_ERRNO("malloc");
    goto err_malloc;
  }

  param->fd = fd;
  param->param.func = func;
  param->param.data = data;

  if (run_task(add_timer_cb, param) < 0)
    goto err_run_task;

  return fd;

err_run_task:
  free(param);
err_malloc:
err_timerfd_settime:
  if (TEMP_FAILURE_RETRY(close(fd)) < 0)
    ALOGW_ERRNO("close");
  return -1;
}

int
add_relative_timer_to_epoll_loop(int clockid,
                                 unsigned long long timeout_ms,
                                 unsigned long long interval_ms,
                                 enum ioresult (*func)(int, uint32_t, void*),
                                 void* data)
{
  assert(interval_ms);

  return add_timer(clockid, 0, timeout_ms, interval_ms, func, data);
}

int
add_absolute_timer_to_epoll_loop(int clockid,
                                 unsigned long long timeout_ms,
                                 unsigned long long interval_ms,
                                 enum ioresult (*func)(int, uint32_t, void*),
                                 void* data)
{
  assert(timeout_ms);

  return add_timer(clockid, 1, timeout_ms, interval_ms, func, data);
}

void
remove_timer(int timer)
{
  if (run_task(remove_timer_cb, (void*)timer) < 0)
    goto err_run_task;

  return;

err_run_task:
  /* If anything goes wrong, we cleanup here and hope for the best.
   */
  remove_fd_from_epoll_loop(timer);
  if (TEMP_FAILURE_RETRY(close(timer)) < 0)
    ALOGW_ERRNO("close");
  return;
}
