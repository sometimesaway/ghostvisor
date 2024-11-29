#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hypercall.h"
#include "util.h"
#include "vm.h"

#define MAX_LOG_SIZE 1024

typedef struct {
    uint64_t memory_size;
    uint32_t vcpu_count;
    uint32_t features;
} vm_info_t;

typedef int (*hypercall_handler_t)(hypercall_regs_t *regs);

static hypercall_handler_t hypercall_handlers[MAX_HYPERCALL];

static int handle_log(hypercall_regs_t *regs) {
    char *msg = (char *)regs->arg1;
    size_t len = regs->arg2;
    
    if (len > MAX_LOG_SIZE) {
        log_error("Guest log message too long: %zu bytes", len);
        return -1;
    }

    char *safe_msg = malloc(len + 1);
    if (!safe_msg) return -1;
    
    if (vm_read_memory(msg, safe_msg, len) != 0) {
        free(safe_msg);
        return -1;
    }
    safe_msg[len] = '\0';

    log_info("Guest: %s", safe_msg);
    free(safe_msg);
    return 0;
}

static int handle_query_info(hypercall_regs_t *regs) {
    vm_info_t *info = (vm_info_t *)regs->arg1;
    vm_info_t local_info;

    if (vm_get_info(&local_info) != 0) {
        return -1;
    }

    if (vm_write_memory(&local_info, info, sizeof(vm_info_t)) != 0) {
        return -1;
    }

    return 0;
}

static int handle_map_memory(hypercall_regs_t *regs) {
    uint64_t guest_addr = regs->arg1;
    uint64_t size = regs->arg2;
    uint32_t flags = regs->arg3;

    return vm_map_memory(guest_addr, size, flags);
}

static int handle_unmap_memory(hypercall_regs_t *regs) {
    uint64_t guest_addr = regs->arg1;
    uint64_t size = regs->arg2;

    return vm_unmap_memory(guest_addr, size);
}

static int handle_register_irq(hypercall_regs_t *regs) {
    uint32_t irq = regs->arg1;
    uint64_t handler = regs->arg2;

    return vm_register_irq_handler(irq, handler);
}

int hypercall_init(void) {
    log_info("Initializing hypercall subsystem...");

    memset(hypercall_handlers, 0, sizeof(hypercall_handlers));

    hypercall_handlers[HYPERCALL_LOG] = handle_log;
    hypercall_handlers[HYPERCALL_QUERY_INFO] = handle_query_info;
    hypercall_handlers[HYPERCALL_MAP_MEMORY] = handle_map_memory;
    hypercall_handlers[HYPERCALL_UNMAP_MEMORY] = handle_unmap_memory;
    hypercall_handlers[HYPERCALL_REGISTER_IRQ] = handle_register_irq;

    return 0;
}

int handle_hypercall(hypercall_regs_t *regs) {
    if (!regs) {
        log_error("Invalid hypercall registers");
        return -1;
    }

    if (regs->nr >= MAX_HYPERCALL || !hypercall_handlers[regs->nr]) {
        log_error("Invalid hypercall number: %llu", regs->nr);
        return -1;
    }

    log_debug("Handling hypercall %llu", regs->nr);
    return hypercall_handlers[regs->nr](regs);
}

void hypercall_cleanup(void) {
    log_info("Cleaning up hypercall subsystem...");
    memset(hypercall_handlers, 0, sizeof(hypercall_handlers));
}
