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

#include "sys/timerfd.h"
#include <sys/syscall.h>

/* The C library's implementation of the timerfd API just moves
 * the arguments into the right places and triggers the system
 * call. We can do the same by using the syscall function. The
 * __NR_ constants are available on all platforms, except some
 * ancient releases of ICS and JB.
 */

#if ANDROID_VERSION <= 16

/* Detect target architecture */
#if __GNUC__
#if __arm__
#define ARCH_ARM
#elif __i386__
#define ARCH_X86
#elif __mips__
#define ARCH_MIPS
#else
#error "Target architecture missing"
#endif
#endif

#ifdef ARCH_ARM

#ifndef __NR_timerfd_create
#define __NR_timerfd_create (__NR_SYSCALL_BASE+350)
#endif /* __NR_timerfd_create */

#ifndef __NR_timerfd_settime
#define __NR_timerfd_settime (__NR_SYSCALL_BASE+353)
#endif /* __NR_timerfd_settime */

#ifndef __NR_timerfd_gettime
#define __NR_timerfd_gettime (__NR_SYSCALL_BASE+354)
#endif /* __NR_timerfd_gettime */

#elif ARCH_MIPS

/* MIPS has get and set reversed */

#ifndef __NR_timerfd_create
#define __NR_timerfd_create (__NR_SYSCALL_BASE+284)
#endif /* __NR_timerfd_create */

#ifndef __NR_timerfd_gettime
#define __NR_timerfd_gettime (__NR_SYSCALL_BASE+285)
#endif /* __NR_timerfd_gettime */

#ifndef __NR_timerfd_settime
#define __NR_timerfd_settime (__NR_SYSCALL_BASE+286)
#endif /* __NR_timerfd_settime */

#elif ARCH_X86

#ifndef __NR_timerfd_create
#define __NR_timerfd_create (__NR_SYSCALL_BASE+322)
#endif /* __NR_timerfd_create */

#ifndef __NR_timerfd_settime
#define __NR_timerfd_settime (__NR_SYSCALL_BASE+325)
#endif /* __NR_timerfd_settime */

#ifndef __NR_timerfd_gettime
#define __NR_timerfd_gettime (__NR_SYSCALL_BASE+326)
#endif /* __NR_timerfd_gettime */

#else
#error "Syscall numbers for timerfd missing"
#endif

#endif /* ANDROID_VERSION <= 16 */

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
