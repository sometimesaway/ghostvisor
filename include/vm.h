#ifndef VM_H
#define VM_H

#include <stdint.h>

typedef struct {
    uint64_t memory_size;  
    int cpu_count;         
} vm_config_t;

int vm_init(void);
int vm_start(vm_config_t *config);
int vm_poll(void);
void vm_stop(void);
void vm_cleanup(void);

#endif // VM_H
