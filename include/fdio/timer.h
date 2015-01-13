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
 * Call |add_timer_to_epoll_loop| to install a timer. The return value
 * is either the timer's file descriptor, or -1 in the case of an error.
 * The parameter |clockid| is the system clock for the timer, as given
 * in 'man timerfd_create'. A non-zero value for the boolean parameter
 * |periodic| sets the timer to fire in periodic intervals, or otherwise
 * fire just once. The parameters |timeout_ms| and |frequency_ms| set
 * the timers start and frequency. If |timeout_ms| is non-zero, the
 * timer will first fire at the *absolute* time given in |timeout_ms|.
 * The timer will further fire with the frequency given in |frequency_ms|
 * if periodic is non-null. If |timeout_ms| is zero, the timer will
 * fire with the frequency given in |frequency_ms| starting at the current
 * time. The parameters |func| and |data| set the call-back function and
 * user data. All timers will fire from within the I/O thread.
 *
 * To remove an existing timer, call |remove_timer|. The parameter is
 * a timer that has been returned by |add_timer_to_epoll_loop|. The
 * function will remove the timer from the I/O loop and cleanup its
 * resources.
 *
 * The implementation currently requires Android API version 19 or later.
 */

#include <stdint.h>
#include "ioresult.h"

#ifdef __cplusplus
extern "C" {
#endif

int
add_timer_to_epoll_loop(int clockid, int periodic,
                        unsigned long long timeout_ms,
                        unsigned long long frequency_ms,
                        enum ioresult (*func)(int, uint32_t, void*),
                        void* data);

void
remove_timer(int timer);

#ifdef __cplusplus
}
#endif
