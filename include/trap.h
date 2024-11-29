#ifndef TRAP_H
#define TRAP_H

#include <stdint.h>

typedef enum {
    TRAP_SYSCALL,
    TRAP_MEMORY,
    TRAP_EXCEPTION
} trap_type_t;

typedef struct {
    trap_type_t type;
    uint64_t address;
    uint64_t data;
} trap_event_t;

int trap_init(void);
int trap_wait_for_event(trap_event_t *event);
void trap_cleanup(void);

#endif // TRAP_H
