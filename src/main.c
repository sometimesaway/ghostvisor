#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "vm.h"
#include "hook.h"
#include "trap.h"
#include "util.h"

static int running = 1;

void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        log_info("Signal received, shutting down...");
        running = 0;
    }
}

int main(int argc, char **argv) {
    log_info("Starting Ghostvisor...");

    if (signal(SIGINT, handle_signal) == SIG_ERR || 
        signal(SIGTERM, handle_signal) == SIG_ERR) {
        log_error("Failed to set up signal handlers.");
        return EXIT_FAILURE;
    }

    if (vm_init() != 0) {
        log_error("Failed to initialize VM subsystem.");
        return EXIT_FAILURE;
    }
    log_info("VM subsystem initialized.");

    if (hook_init() != 0) {
        log_error("Failed to initialize hooks.");
        vm_cleanup();
        return EXIT_FAILURE;
    }
    log_info("Hooking subsystem initialized.");

    if (trap_init() != 0) {
        log_error("Failed to initialize exception handling.");
        hook_cleanup();
        vm_cleanup();
        return EXIT_FAILURE;
    }
    log_info("Exception handling subsystem initialized.");

    while (running) {
        if (vm_poll() != 0) {
            log_error("Error occurred while polling VM events.");
            break;
        }
    }

    log_info("Shutting down Ghostvisor...");
    trap_cleanup();
    hook_cleanup();
    vm_cleanup();
    log_info("Ghostvisor stopped cleanly.");

    return EXIT_SUCCESS;
}
