#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "trap.h"

typedef void (*event_handler_t)(const trap_event_t *event);

typedef enum {
    EVENT_SYSCALL = TRAP_SYSCALL,
    EVENT_MEMORY = TRAP_MEMORY,
    EVENT_EXCEPTION = TRAP_EXCEPTION
} event_type_t;

int register_event_handler(event_type_t type, event_handler_t handler);
void dispatch_event(const trap_event_t *event);

#endif // DISPATCHER_H
