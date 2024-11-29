#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hook.h"
#include "util.h"

static int hook_initialized = 0;
static hook_handler_t *syscall_handlers = NULL;
static hook_handler_t *memory_handlers = NULL;
static hook_handler_t *exception_handlers = NULL;

int hook_init(void) {
    if (hook_initialized) {
        log_warn("Hook subsystem already initialized.");
        return 0;
    }
    
    log_info("Initializing hooking subsystem...");
    
    syscall_handlers = calloc(MAX_SYSCALL_HANDLERS, sizeof(hook_handler_t));
    memory_handlers = calloc(MAX_MEMORY_HANDLERS, sizeof(hook_handler_t)); 
    exception_handlers = calloc(MAX_EXCEPTION_HANDLERS, sizeof(hook_handler_t));
    
    if (!syscall_handlers || !memory_handlers || !exception_handlers) {
        log_error("Failed to allocate handler arrays");
        hook_cleanup();
        return -1;
    }

    hook_initialized = 1;
    return 0;
}

int handle_syscall(const trap_event_t *event) {
    if (!hook_initialized) {
        log_error("Hook subsystem not initialized.");
        return -1;
    }

    log_debug("Handling syscall trap, number: %llu", event->data);

    // Look for registered handlers for this syscall
    for (int i = 0; i < MAX_SYSCALL_HANDLERS; i++) {
        if (syscall_handlers[i].type == HOOK_TYPE_SYSCALL && 
            syscall_handlers[i].id == event->data &&
            syscall_handlers[i].handler) {
            
            return syscall_handlers[i].handler(event);
        }
    }

    log_warn("No handler found for syscall: %llu", event->data);
    return -1;
}

int handle_memory_access(const trap_event_t *event) {
    if (!hook_initialized) {
        log_error("Hook subsystem not initialized.");
        return -1;
    }

    log_debug("Handling memory access trap at address: 0x%llx", event->address);

    for (int i = 0; i < MAX_MEMORY_HANDLERS; i++) {
        if (memory_handlers[i].type == HOOK_TYPE_MEMORY &&
            event->address >= memory_handlers[i].region_start &&
            event->address <= memory_handlers[i].region_end &&
            memory_handlers[i].handler) {
            
            return memory_handlers[i].handler(event);
        }
    }

    log_warn("No handler found for memory access at: 0x%llx", event->address);
    return 0; 
}

int handle_exception(const trap_event_t *event) {
    if (!hook_initialized) {
        log_error("Hook subsystem not initialized.");
        return -1;
    }

    log_debug("Handling exception trap, code: 0x%llx", event->data);

    for (int i = 0; i < MAX_EXCEPTION_HANDLERS; i++) {
        if (exception_handlers[i].type == HOOK_TYPE_EXCEPTION &&
            exception_handlers[i].id == event->data &&
            exception_handlers[i].handler) {
            
            return exception_handlers[i].handler(event);
        }
    }

    log_error("No handler found for exception: 0x%llx", event->data);
    return -1;
}

int register_hook(hook_type_t type, uint64_t id, 
                 uint64_t region_start, uint64_t region_end,
                 hook_handler_func_t handler) {
    
    if (!hook_initialized) {
        log_error("Hook subsystem not initialized.");
        return -1;
    }

    hook_handler_t *handlers;
    int max_handlers;

    switch(type) {
        case HOOK_TYPE_SYSCALL:
            handlers = syscall_handlers;
            max_handlers = MAX_SYSCALL_HANDLERS;
            break;
        case HOOK_TYPE_MEMORY:
            handlers = memory_handlers; 
            max_handlers = MAX_MEMORY_HANDLERS;
            break;
        case HOOK_TYPE_EXCEPTION:
            handlers = exception_handlers;
            max_handlers = MAX_EXCEPTION_HANDLERS;
            break;
        default:
            log_error("Invalid hook type: %d", type);
            return -1;
    }

    for (int i = 0; i < max_handlers; i++) {
        if (!handlers[i].handler) {
            handlers[i].type = type;
            handlers[i].id = id;
            handlers[i].region_start = region_start;
            handlers[i].region_end = region_end;
            handlers[i].handler = handler;
            return 0;
        }
    }

    log_error("No free handler slots for type: %d", type);
    return -1;
}

void hook_cleanup(void) {
    if (!hook_initialized) {
        log_warn("Hook subsystem not initialized, nothing to clean up.");
        return;
    }

    log_info("Cleaning up hooking subsystem...");
    
    free(syscall_handlers);
    free(memory_handlers);
    free(exception_handlers);
    
    syscall_handlers = NULL;
    memory_handlers = NULL;
    exception_handlers = NULL;
    
    hook_initialized = 0;
}
