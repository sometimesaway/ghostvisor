# ghostvisor

Minimal arm64 type-1 hypervisor with async trap handling and dynamic hook support.

## Architecture

Uses a multi-threaded architecture to decouple trap handling from the main VM execution flow:

- **Async Trap Processing**: Trap events (syscalls, memory access, exceptions) are processed by a dedicated thread pool
- **Dynamic Hook System**: Runtime-loadable hooks for system calls, memory regions, and exception handlers
- **Non-blocking VM Execution**: Main VM thread remains responsive while traps are handled asynchronously

## Building

Requires:
- GCC/Clang with C11 support
- pthread
- dlopen support

```bash
make
```

## Usage

```c
vm_config_t config = {
    .cpu_count = 4,
    .memory_size = 1024 * 1024 * 1024  // 1GB
};
vm_init();
vm_start(&config);

register_dynamic_hook(
    "hooks/syscall_hook.so",     
    "handle_syscall",           
    HOOK_TYPE_SYSCALL,       
    SYS_write,                 
    0, 0                       
);

vm_poll();
```

