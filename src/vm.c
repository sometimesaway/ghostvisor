#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "trap.h"
#include "hook.h"
#include "util.h"

typedef struct {
    uint64_t *memory;
    int running;
    int vcpu_count;
} vm_state_t;

static vm_state_t vm = {0};

int vm_init(void) {
    log_info("Initializing VM subsystem...");
    memset(&vm, 0, sizeof(vm_state_t));
    return 0;
}

int vm_start(vm_config_t *config) {
    if (vm.running) {
        log_error("VM is already running.");
        return -1;
    }
    vm.memory = (uint64_t *)malloc(config->memory_size);
    if (!vm.memory) {
        log_error("Failed to allocate guest memory.");
        return -1;
    }
    memset(vm.memory, 0, config->memory_size);
    vm.vcpu_count = config->cpu_count;
    vm.running = 1;
    log_info("VM started with %d vCPUs and %llu bytes of memory.", vm.vcpu_count, config->memory_size);
    return 0;
}

int vm_poll(void) {
    if (!vm.running) {
        log_error("VM is not running.");
        return -1;
    }
    log_debug("Polling VM events...");
    trap_event_t event;
    while (trap_wait_for_event(&event) == 0) {
        switch (event.type) {
            case TRAP_SYSCALL:
                log_info("Intercepted syscall from guest.");
                if (handle_syscall(&event) != 0) {
                    log_error("Failed to handle syscall.");
                    return -1;
                }
                break;
            case TRAP_MEMORY:
                log_info("Intercepted memory access from guest.");
                if (handle_memory_access(&event) != 0) {
                    log_error("Failed to handle memory access.");
                    return -1;
                }
                break;
            case TRAP_EXCEPTION:
                log_info("Intercepted exception from guest.");
                if (handle_exception(&event) != 0) {
                    log_error("Failed to handle exception.");
                    return -1;
                }
                break;
            default:
                log_warn("Unknown trap type encountered: %d", event.type);
                break;
        }
    }
    return 0;
}

void vm_stop(void) {
    if (!vm.running) {
        log_error("VM is not running.");
        return;
    }
    log_info("Stopping VM...");
    free(vm.memory);
    vm.memory = NULL;
    vm.running = 0;
    vm.vcpu_count = 0;
    log_info("VM stopped.");
}

void vm_cleanup(void) {
    if (vm.running) {
        log_warn("VM is still running during cleanup. Stopping it now...");
        vm_stop();
    }
    log_info("VM subsystem cleaned up.");
}
