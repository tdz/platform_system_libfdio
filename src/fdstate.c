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
#include "log.h"
#include "fdstate.h"

static size_t nfdstates;
static struct fd_state* fd_state;

static unsigned long
ceilpow2(unsigned long l)
{
  size_t bits, i;

  bits = sizeof(l) * CHAR_BIT;

  --l;
  for (i = 1; i < bits; i *= 2) {
    l |= l >> i;
  }
  ++l;

  return l + !l; /* return next highest power of 2, or 1 for zero */
}

struct fd_state*
get_fd_state(int fd, int alloc)
{
  size_t newnfdstates;
  void* mem;
  size_t i;

  assert(fd >= 0);

  /* return current entry */

  if ((size_t)fd < nfdstates) {
    return fd_state + fd;
  } else if (!alloc) {
    return NULL;
  }

  /* or grow fd_state array */

  newnfdstates = ceilpow2(fd + 1);
  if (newnfdstates <= (size_t)fd) {
    ALOGE("overflow detected %zu %d", newnfdstates, fd);
    return NULL;
  }

  errno = 0;
  mem = realloc(fd_state, newnfdstates * sizeof(*fd_state));
  if (errno) {
    ALOGE_ERRNO("realloc");
    return NULL;
  }
  fd_state = mem;

  for (i = nfdstates; i < newnfdstates; ++i) {
    clear_fd_state(fd_state + i);
  }
  nfdstates = newnfdstates;

  return fd_state + fd;
}

void
clear_fd_state(struct fd_state* fd_state)
{
  assert(fd_state);

  fd_state->event.events = 0;
  /* use the largest union member to clean allocated memory */
  fd_state->event.data.u64 = 0;
  fd_state->func = NULL;
  fd_state->data = NULL;
}

void
free_fd_states()
{
  free(fd_state);
  nfdstates = 0;
}
