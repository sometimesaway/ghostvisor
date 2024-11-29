#ifndef HOOK_H
#define HOOK_H

#include "trap.h"

int hook_init(void);
int handle_syscall(const trap_event_t *event);
int handle_memory_access(const trap_event_t *event);
int handle_exception(const trap_event_t *event);
void hook_cleanup(void);

#endif // HOOK_H
