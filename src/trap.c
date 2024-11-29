#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include "trap.h"
#include "util.h"

typedef struct {
    int initialized;
    void *trap_page;
    size_t trap_page_size;
    trap_handler_t handler;
    trap_event_queue_t *queue;
    pthread_mutex_t lock;
} trap_state_t;

static trap_state_t trap_state = {0};

int trap_init(void) {
    if (trap_state.initialized) {
        log_warn("Trap subsystem already initialized.");
        return 0;
    }

    log_info("Initializing trapping subsystem...");
    
    if (pthread_mutex_init(&trap_state.lock, NULL) != 0) {
        log_error("Failed to initialize trap mutex: %s", strerror(errno));
        return -1;
    }

    trap_state.trap_page_size = sysconf(_SC_PAGESIZE);
    trap_state.trap_page = mmap(NULL, trap_state.trap_page_size,
                               PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (trap_state.trap_page == MAP_FAILED) {
        log_error("Failed to allocate trap page: %s", strerror(errno));
        pthread_mutex_destroy(&trap_state.lock);
        return -1;
    }

    trap_state.queue = trap_queue_create(MAX_TRAP_EVENTS);
    if (!trap_state.queue) {
        log_error("Failed to create trap event queue");
        munmap(trap_state.trap_page, trap_state.trap_page_size);
        pthread_mutex_destroy(&trap_state.lock);
        return -1;
    }

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = trap_signal_handler;

    if (sigaction(SIGSEGV, &sa, NULL) == -1 ||
        sigaction(SIGBUS, &sa, NULL) == -1 ||
        sigaction(SIGILL, &sa, NULL) == -1) {
        log_error("Failed to register signal handlers: %s", strerror(errno));
        trap_queue_destroy(trap_state.queue);
        munmap(trap_state.trap_page, trap_state.trap_page_size);
        pthread_mutex_destroy(&trap_state.lock);
        return -1;
    }

    trap_state.initialized = 1;
    return 0;
}

int trap_wait_for_event(trap_event_t *event) {
    if (!trap_state.initialized) {
        log_error("Trap subsystem not initialized.");
        return -1;
    }

    if (!event) {
        log_error("Invalid event pointer provided.");
        return -1;
    }

    pthread_mutex_lock(&trap_state.lock);

    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += TRAP_WAIT_TIMEOUT_SEC;

    int result = trap_queue_wait_and_pop(trap_state.queue, event, &timeout);
    
    if (result == 0) {
        switch (event->type) {
            case TRAP_TYPE_PAGE_FAULT:
                handle_page_fault(event);
                break;
            case TRAP_TYPE_INVALID_INSTRUCTION:
                handle_invalid_instruction(event);
                break;
            case TRAP_TYPE_DIVISION_ERROR:
                handle_division_error(event);
                break;
            default:
                log_warn("Unknown trap type: %d", event->type);
                result = -1;
        }
    } else if (result == ETIMEDOUT) {
        log_debug("Trap wait timeout reached");
        result = 1;
    } else {
        log_error("Error waiting for trap event: %s", strerror(errno));
        result = -1;
    }

    pthread_mutex_unlock(&trap_state.lock);

    if (result == 0) {
        log_debug("Trap event received: type=%d, address=0x%llx, data=0x%llx",
                  event->type, event->address, event->data);
    }

    return result;
}

void trap_cleanup(void) {
    if (!trap_state.initialized) {
        log_warn("Trap subsystem not initialized, nothing to clean up.");
        return;
    }

    log_info("Cleaning up trapping subsystem...");

    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGILL, SIG_DFL);

    if (trap_state.queue) {
        trap_queue_destroy(trap_state.queue);
    }

    if (trap_state.trap_page) {
        munmap(trap_state.trap_page, trap_state.trap_page_size);
    }

    pthread_mutex_destroy(&trap_state.lock);
    trap_state.initialized = 0;
}
