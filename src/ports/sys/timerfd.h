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

#pragma once

#if ANDROID_VERSION >= 19

#include <sys/timerfd.h>

#else

/*
 * See 'man 2 timerfd_create' for documentation of these functions.
 */

#include <time.h>
#include <fcntl.h>
#include <sys/types.h>

/* defined by kernel in <include/linux/timerfd.h> */
#define TFD_TIMER_ABSTIME (1 << 0)
#define TFD_TIMER_CANCEL_ON_SET (1 << 1)
#define TFD_CLOEXEC O_CLOEXEC
#define TFD_NONBLOCK O_NONBLOCK

int
timerfd_create(int clockid, int flags);

int
timerfd_settime(int fd, int flags,
                const struct itimerspec* new_value,
                struct itimerspec* old_value);

int
timerfd_gettime(int fd, struct itimerspec* curr_value);

#endif
