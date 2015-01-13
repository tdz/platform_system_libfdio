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

#if ANDROID_VERSION >= 19
#include <assert.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#else
#include <stdlib.h>
#endif
#include <fdio/loop.h>
#include <fdio/task.h>
#include <fdio/timer.h>
#include "compiler.h"
#include "log.h"

#if ANDROID_VERSION >= 19
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

  add_fd_to_epoll_loop(param->fd, EPOLLIN|EPOLLERR,
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
#endif

int
add_timer_to_epoll_loop(int clockid ATTRIBS(UNUSED),
                        int periodic ATTRIBS(UNUSED),
                        unsigned long long timeout_ms ATTRIBS(UNUSED),
                        unsigned long long frequency_ms ATTRIBS(UNUSED),
                        enum ioresult (*func)(int, uint32_t, void*) ATTRIBS(UNUSED),
                        void* data ATTRIBS(UNUSED))
{
#if ANDROID_VERSION >= 19
  int fd, flags;
  struct itimerspec timeout;
  struct add_timer_param* param;

  assert(timeout_ms || frequency_ms);

  fd = timerfd_create(clockid, TFD_NONBLOCK|TFD_CLOEXEC);
  if (fd < 0) {
    ALOGE_ERRNO("timerfd_create");
    return -1;
  }

  if (periodic) {
    timeout.it_interval.tv_sec = 0;
    timeout.it_interval.tv_nsec = 0;
  } else {
    timeout.it_interval.tv_sec = frequency_ms / 1000;
    timeout.it_interval.tv_nsec = (frequency_ms % 1000) * 1000000;
  }

  if (timeout_ms) {
    flags = TFD_TIMER_ABSTIME;
    timeout.it_value.tv_sec = timeout_ms / 1000;
    timeout.it_value.tv_nsec = (timeout_ms % 1000) * 1000000;
  } else {
    flags = 0;
    timeout.it_value.tv_sec = frequency_ms / 1000;
    timeout.it_value.tv_nsec = (frequency_ms % 1000) * 1000000;
  }

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
#else
  ALOGE("add_timer_to_epoll_loop is not implemented on "
        "Android before API version 19.");
  abort();
#endif
}

void
remove_timer(int timer ATTRIBS(UNUSED))
{
#if ANDROID_VERSION >= 19
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
#else
  ALOGE("remove_timer is not implemented on "
        "Android before API version 19.");
  abort();
#endif
}
