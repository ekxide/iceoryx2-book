#include "iox2/iceoryx2.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// stand-ins for Larry's sensor hardware
static bool bump_sensor_was_activated(void) {
    return false;
}

static float battery_state(void) {
    return 100.0F; // NOLINT
}

#define BATTERY_THRESHOLD 20.0F

int main(void) {
    // snippet:start node-and-service
    iox2_node_builder_h node_builder_handle = iox2_node_builder_new(NULL);
    iox2_node_h node = NULL;
    if (iox2_node_builder_create(node_builder_handle, NULL, iox2_service_type_e_IPC, &node) != IOX2_OK) {
        printf("Could not create node!\n");
        exit(-1);
    }

    const char* service_name_value = "system_health_events";
    iox2_service_name_h service_name = NULL;
    if (iox2_service_name_new(NULL, service_name_value, strlen(service_name_value), &service_name) != IOX2_OK) {
        printf("Unable to create service name!\n");
        exit(-1);
    }

    iox2_service_name_ptr service_name_ptr = iox2_cast_service_name_ptr(service_name);
    iox2_service_builder_h service_builder = iox2_node_service_builder(&node, NULL, service_name_ptr);
    iox2_service_builder_event_h service_builder_event = iox2_service_builder_event(service_builder);
    iox2_port_factory_event_h service = NULL;
    if (iox2_service_builder_event_open_or_create(service_builder_event, NULL, &service) != IOX2_OK) {
        printf("Unable to create service!\n");
        exit(-1);
    }
    // snippet:end node-and-service

    // snippet:start notifier
    iox2_port_factory_notifier_builder_h notifier_builder = iox2_port_factory_event_notifier_builder(&service, NULL);
    iox2_notifier_h notifier = NULL;
    if (iox2_port_factory_notifier_builder_create(notifier_builder, NULL, &notifier) != IOX2_OK) {
        printf("Unable to create notifier!\n");
        exit(-1);
    }
    // snippet:end notifier

    // snippet:start event-ids
    iox2_event_id_t wall_was_hit = { .value = 0 };
    iox2_event_id_t battery_is_low = { .value = 1 };
    // snippet:end event-ids

    // snippet:start notify-loop
    while (iox2_node_wait(&node, 1, 0) == IOX2_OK) {
        if (bump_sensor_was_activated()) {
            if (iox2_notifier_notify_with_custom_event_id(&notifier, &wall_was_hit, NULL) != IOX2_OK) {
                printf("Failed to notify listener!\n");
                exit(-1);
            }
        }

        if (battery_state() < BATTERY_THRESHOLD) {
            if (iox2_notifier_notify_with_custom_event_id(&notifier, &battery_is_low, NULL) != IOX2_OK) {
                printf("Failed to notify listener!\n");
                exit(-1);
            }
        }
    }

    // release every handle once the loop exits
    iox2_notifier_drop(notifier);
    iox2_port_factory_event_drop(service);
    iox2_service_name_drop(service_name);
    iox2_node_drop(node);
    // snippet:end notify-loop

    return 0;
}
