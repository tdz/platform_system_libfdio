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

#include <sys/syscall.h>
#include "sys/timerfd.h"

/* The C library's implementation of the timerfd API just moves
 * the arguments into the right places and triggers the system
 * call. We can do the same by using the syscall function. The
 * __NR_ constants are available on all platforms.
 */

int
timerfd_create(int clockid, int flags)
{
  return syscall(__NR_timerfd_create, clockid, flags, 0, 0, 0, 0);
}

int
timerfd_settime(int fd, int flags,
                const struct itimerspec* new_value,
                struct itimerspec* old_value)
{
  return syscall(__NR_timerfd_settime, fd, flags, new_value, old_value, 0, 0);
}

int
timerfd_gettime(int fd, struct itimerspec* curr_value)
{
  return syscall(__NR_timerfd_gettime, fd, curr_value);
}
