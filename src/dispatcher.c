#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dispatcher.h"

#define MAX_EVENT_HANDLERS 10

typedef struct {
    event_type_t type;
    event_handler_t handler;
} event_handler_entry_t;

static event_handler_entry_t event_handlers[MAX_EVENT_HANDLERS];
static int handler_count = 0;

int register_event_handler(event_type_t type, event_handler_t handler) {
    if (handler_count >= MAX_EVENT_HANDLERS) {
        fprintf(stderr, "Max event handlers reached\n");
        return -1;
    }
    event_handlers[handler_count].type = type;
    event_handlers[handler_count].handler = handler;
    handler_count++;
    return 0;
}

void dispatch_event(const trap_event_t *event) {
    for (int i = 0; i < handler_count; i++) {
        if (event_handlers[i].type == event->type) {
            event_handlers[i].handler(event);
            return;
        }
    }
    fprintf(stderr, "No handler for event type: %d\n", event->type);
}
