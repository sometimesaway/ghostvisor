#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "thread_pool.h"
#include "hook.h"
#include "util.h"

#define MAX_QUEUE_SIZE 1024

typedef struct {
    trap_event_t event;
    int valid;
} work_item_t;

typedef struct {
    work_item_t *queue;
    int head;
    int tail;
    int count;
    int size;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    int shutdown;
} work_queue_t;

struct thread_pool {
    pthread_t *threads;
    int num_threads;
    work_queue_t queue;
};

static work_queue_t *work_queue_create(int size) {
    work_queue_t *queue = calloc(1, sizeof(work_queue_t));
    if (!queue) return NULL;

    queue->queue = calloc(size, sizeof(work_item_t));
    if (!queue->queue) {
        free(queue);
        return NULL;
    }

    queue->size = size;
    queue->head = queue->tail = queue->count = 0;
    queue->shutdown = 0;

    if (pthread_mutex_init(&queue->lock, NULL) != 0) {
        free(queue->queue);
        free(queue);
        return NULL;
    }

    if (pthread_cond_init(&queue->not_empty, NULL) != 0 ||
        pthread_cond_init(&queue->not_full, NULL) != 0) {
        pthread_mutex_destroy(&queue->lock);
        free(queue->queue);
        free(queue);
        return NULL;
    }

    return queue;
}

static void work_queue_destroy(work_queue_t *queue) {
    if (!queue) return;
    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
    free(queue->queue);
    free(queue);
}

static void *worker_thread(void *arg) {
    thread_pool_t *pool = (thread_pool_t *)arg;
    work_queue_t *queue = &pool->queue;

    while (1) {
        pthread_mutex_lock(&queue->lock);

        while (queue->count == 0 && !queue->shutdown) {
            pthread_cond_wait(&queue->not_empty, &queue->lock);
        }

        if (queue->shutdown && queue->count == 0) {
            pthread_mutex_unlock(&queue->lock);
            pthread_exit(NULL);
        }

        // Get work item
        work_item_t item = queue->queue[queue->head];
        queue->head = (queue->head + 1) % queue->size;
        queue->count--;

        pthread_cond_signal(&queue->not_full);
        pthread_mutex_unlock(&queue->lock);

        if (item.valid) {
            // Process trap event based on type
            switch (item.event.type) {
                case TRAP_SYSCALL:
                    handle_syscall(&item.event);
                    break;
                case TRAP_MEMORY:
                    handle_memory_access(&item.event);
                    break;
                case TRAP_EXCEPTION:
                    handle_exception(&item.event);
                    break;
                default:
                    log_error("Unknown trap type: %d", item.event.type);
            }
        }
    }

    return NULL;
}

thread_pool_t *thread_pool_create(int num_threads) {
    thread_pool_t *pool = calloc(1, sizeof(thread_pool_t));
    if (!pool) return NULL;

    pool->num_threads = num_threads;
    pool->threads = calloc(num_threads, sizeof(pthread_t));
    if (!pool->threads) {
        free(pool);
        return NULL;
    }

    work_queue_t *queue = work_queue_create(MAX_QUEUE_SIZE);
    if (!queue) {
        free(pool->threads);
        free(pool);
        return NULL;
    }

    memcpy(&pool->queue, queue, sizeof(work_queue_t));
    free(queue);

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
            thread_pool_destroy(pool);
            return NULL;
        }
    }

    return pool;
}

int thread_pool_submit(thread_pool_t *pool, trap_event_t *event) {
    if (!pool || !event) return -1;

    work_queue_t *queue = &pool->queue;
    pthread_mutex_lock(&queue->lock);

    while (queue->count == queue->size && !queue->shutdown) {
        pthread_cond_wait(&queue->not_full, &queue->lock);
    }

    if (queue->shutdown) {
        pthread_mutex_unlock(&queue->lock);
        return -1;
    }

    work_item_t *item = &queue->queue[queue->tail];
    item->event = *event;
    item->valid = 1;

    queue->tail = (queue->tail + 1) % queue->size;
    queue->count++;

    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->lock);

    return 0;
}

void thread_pool_destroy(thread_pool_t *pool) {
    if (!pool) return;

    work_queue_t *queue = &pool->queue;
    pthread_mutex_lock(&queue->lock);
    queue->shutdown = 1;
    pthread_cond_broadcast(&queue->not_empty);
    pthread_mutex_unlock(&queue->lock);

    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    work_queue_destroy(&pool->queue);
    free(pool->threads);
    free(pool);
}
