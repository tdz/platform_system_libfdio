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

/*
 * This is a simple timer API for libfdio.
 *
 * Call |add_relative_timer_to_epoll_loop| to install a timer that fires
 * relative to the current time. The return value is either the timer's
 * file descriptor, or -1 in the case of an error.
 *
 * The parameter |clockid| is the system clock for the timer, as given
 * in 'man timerfd_create'. The parameter |timeout_ms| sets the timeout
 * of the timer; relative to the current time. The parameter |interval_ms|
 * sets the timer's interval _after_ the timer first fired. If the
 * interval is 0, the timer will fire just once. Both time values are
 * specified in milliseconds. The parameters |func| and |data| set the
 * call-back function and user data. All timers will fire from within
 * the I/O thread.
 *
 * The function |add_absolute_timer_to_epoll_loop| adds an absolute timer
 * to the I/O loop. The parameter |timeout_ms| specifies a timeout at
 * an absolute time. All other parameters are the same as for
 * |add_relative_timer_to_epoll_loop|.
 *
 * To remove an existing timer, call |remove_fd_from_epoll_loop|. The
 * parameter is a timer that has been returned by |add_{relative,absolute}_
 * timer_to_epoll_loop|. The function will remove the timer from the I/O
 * loop. File descriptors must be closed by callers.
 */

#include <stdint.h>
#include "ioresult.h"

#ifdef __cplusplus
extern "C" {
#endif

int
add_relative_timer_to_epoll_loop(int clockid,
                                 unsigned long long timeout_ms,
                                 unsigned long long interval_ms,
                                 enum ioresult (*func)(int, uint32_t, void*),
                                 void* data);

int
add_absolute_timer_to_epoll_loop(int clockid,
                                 unsigned long long timeout_ms,
                                 unsigned long long interval_ms,
                                 enum ioresult (*func)(int, uint32_t, void*),
                                 void* data);

#ifdef __cplusplus
}
#endif
