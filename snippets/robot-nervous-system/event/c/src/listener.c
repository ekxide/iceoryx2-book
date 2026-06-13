#include "iox2/iceoryx2.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// stand-ins for the health-monitor's reactions
static void activate_battery_warning_light(void) {
}

static void go_into_parking_position(void) {
}

// the event IDs the listener reacts to, passed to the wait callback as context
struct reaction_targets {
    iox2_event_id_t battery_is_low;
    iox2_event_id_t wall_was_hit;
};

static void react_to_event(const iox2_event_id_t* event_id, uint64_t count, void* context) {
    (void) count;
    struct reaction_targets* targets = (struct reaction_targets*) context;
    if (event_id->value == targets->battery_is_low.value) {
        activate_battery_warning_light();
    }
    if (event_id->value == targets->wall_was_hit.value) {
        go_into_parking_position();
    }
}

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

    // snippet:start listener
    iox2_port_factory_listener_builder_h listener_builder = iox2_port_factory_event_listener_builder(&service, NULL);
    iox2_listener_h listener = NULL;
    if (iox2_port_factory_listener_builder_create(listener_builder, NULL, &listener) != IOX2_OK) {
        printf("Unable to create listener!\n");
        exit(-1);
    }
    // snippet:end listener

    iox2_event_id_t wall_was_hit = { .value = 0 };
    iox2_event_id_t battery_is_low = { .value = 1 };
    struct reaction_targets targets = { battery_is_low, wall_was_hit };

    // snippet:start wait-loop
    uint64_t number_of_notifications = 0;
    while (iox2_node_wait(&node, 0, 0) == IOX2_OK) {
        if (iox2_listener_blocking_wait(&listener, &number_of_notifications, react_to_event, &targets) != IOX2_OK) {
            printf("Unable to wait for notification!\n");
            exit(-1);
        }
    }

    // release every handle once the loop exits
    iox2_listener_drop(listener);
    iox2_port_factory_event_drop(service);
    iox2_service_name_drop(service_name);
    iox2_node_drop(node);
    // snippet:end wait-loop

    return 0;
}
