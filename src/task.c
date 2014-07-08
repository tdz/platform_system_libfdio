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
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fdio/loop.h>
#include <fdio/task.h>
#include "log.h"

struct task {
  int (*func)(void* data);
  void* data;
};

static int pipefd[2];

static struct task*
fetch_task(void)
{
  union {
    struct task* task;
    unsigned char raw[sizeof(struct task*)];
  } buf;

  size_t off;
  size_t count;

  off = 0;
  count = sizeof(buf.task);

  while (count) {
    ssize_t res = TEMP_FAILURE_RETRY(read(pipefd[0], buf.raw+off, count));
    if (res < 0) {
      /* if this fails, you best close the pipe and restart completely */
      ALOGE_ERRNO("read");
      return NULL;
    }
    count -= res;
    off += res;
  }

  return buf.task;
}

static void
delete_task(struct task* task)
{
  assert(task);
  free(task);
}

static void
exec_task()
{
  struct task* task = fetch_task();
  if (!task)
    return;

  task->func(task->data);
  delete_task(task);
}

static void
task_epollin_cb(int fd, void* data)
{
  exec_task();
}

static void
task_epollerr_cb(int fd, void* data)
{
  ALOGE("Task pipe error");
}

int
init_task_queue()
{
  static const struct fd_events evfuncs = {
    .epollin = task_epollin_cb,
    .epollerr = task_epollerr_cb
  };

  if (TEMP_FAILURE_RETRY(pipe(pipefd)) < 0) {
    ALOGE_ERRNO("pipe");
    return -1;
  }

  if (add_fd_events_to_epoll_loop(pipefd[0], EPOLLIN|EPOLLERR, &evfuncs) < 0)
    goto err_add_fd_events_to_epoll_loop;

  return 0;
err_add_fd_events_to_epoll_loop:
  if (TEMP_FAILURE_RETRY(close(pipefd[1])))
    ALOGW_ERRNO("close");
  if (TEMP_FAILURE_RETRY(close(pipefd[0])))
    ALOGW_ERRNO("close");
  return -1;
}

void
uninit_task_queue()
{
  if (TEMP_FAILURE_RETRY(close(pipefd[1])))
    ALOGW_ERRNO("close");
  if (TEMP_FAILURE_RETRY(close(pipefd[0])))
    ALOGW_ERRNO("close");
}

static int
send_task(struct task* task)
{
  ssize_t res;

  assert(sizeof(task) <= PIPE_BUF); /* guarantee atomicity of pipe writes */

  res = TEMP_FAILURE_RETRY(write(pipefd[1], &task, sizeof(task)));
  if (res < 0) {
    ALOGE_ERRNO("write");
    return -1;
  } else if (res != sizeof(task)) {
    /* your OS is broken */
    abort();
  }

  return 0;
}

static struct task*
create_task(int (*func)(void*), void* data)
{
  struct task* task;

  assert(func);

  errno = 0;
  task = malloc(sizeof(*task));
  if (errno) {
    ALOGE_ERRNO("malloc");
    return NULL;
  }

  task->func = func;
  task->data = data;

  return task;
}

int
run_task(int (*func)(void*), void* data)
{
  struct task* task;

  if (!func) {
    ALOGE("no callback function specified");
    return -1;
  }

  task = create_task(func, data);
  if (!task)
    return -1;

  if (send_task(task) < 0)
    goto err_send_task;

  return 0;
err_send_task:
  delete_task(task);
  return -1;
}
