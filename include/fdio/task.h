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

#pragma once

/*
 * This file provides the API to running a function in the context of the
 * poll loop.
 *
 * The task queue must be initialized using |init_task_queue|. It should
 * be called from within the init function that is supplied to |epoll_loop|.
 *
 * The function |run_task| dispatches an event to the poll loop that executes
 * the function 'func' with the supplied user data as argument.
 *
 * This interface is _not_ thread-safe, except for |run_task|.
 */

#ifdef __cplusplus
extern "C" {
#endif

int
init_task_queue(void);

void
uninit_task_queue(void);

int
run_task(int (*func)(void*), void* data);

#ifdef __cplusplus
}
#endif
