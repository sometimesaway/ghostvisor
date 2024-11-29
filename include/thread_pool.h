#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <stdint.h>
#include "trap.h"

typedef struct thread_pool thread_pool_t;

thread_pool_t *thread_pool_create(int num_threads);

int thread_pool_submit(thread_pool_t *pool, trap_event_t *event);

void thread_pool_destroy(thread_pool_t *pool);

#endif // THREAD_POOL_H
