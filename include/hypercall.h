#ifndef HYPERCALL_H
#define HYPERCALL_H

#include <stdint.h>

typedef enum {
    HYPERCALL_INVALID = 0,
    HYPERCALL_LOG = 1,           
    HYPERCALL_QUERY_INFO = 2,    
    HYPERCALL_MAP_MEMORY = 3,    
    HYPERCALL_UNMAP_MEMORY = 4, 
    HYPERCALL_REGISTER_IRQ = 5,  
    MAX_HYPERCALL
} hypercall_nr_t;

typedef struct {
    uint64_t nr;     
    uint64_t arg1;    
    uint64_t arg2;   
    uint64_t arg3;    
    uint64_t ret;     
} hypercall_regs_t;

int hypercall_init(void);

int handle_hypercall(hypercall_regs_t *regs);

void hypercall_cleanup(void);

#endif // HYPERCALL_H
