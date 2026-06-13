#include "distance.h"
#include "iox2/iceoryx2.h"

#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__) || defined(_WIN64)
#define alignof __alignof
#else
#include <stdalign.h>
#endif
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const iox2_event_id_t ultra_sonic_service_dead = { .value = 10 };
static const iox2_event_id_t obstacle_too_close = { .value = 5 };

// stands in for the emergency-brake reactions
static void go_into_parking_position(void) {
}

static void perform_break(iox2_sample_h* last_samples, size_t count) {
    (void) last_samples;
    (void) count;
}

// records which events fired so the loop can react after waking
struct received_events {
    bool sensor_is_dead;
    bool obstacle_detected;
};

static void collect_event(const iox2_event_id_t* event_id, uint64_t count, void* context) {
    (void) count;
    struct received_events* events = (struct received_events*) context;
    if (event_id->value == ultra_sonic_service_dead.value) {
        events->sensor_is_dead = true;
    }
    if (event_id->value == obstacle_too_close.value) {
        events->obstacle_detected = true;
    }
}

int main(void) {
    // snippet:start setup
    iox2_node_builder_h node_builder_handle = iox2_node_builder_new(NULL);
    iox2_node_h node = NULL;
    if (iox2_node_builder_create(node_builder_handle, NULL, iox2_service_type_e_IPC, &node) != IOX2_OK) {
        printf("Could not create node!\n");
        exit(-1);
    }

    const char* service_name_value = "distance_to_obstacle";
    iox2_service_name_h service_name = NULL;
    if (iox2_service_name_new(NULL, service_name_value, strlen(service_name_value), &service_name) != IOX2_OK) {
        printf("Unable to create service name!\n");
        exit(-1);
    }

    iox2_service_name_ptr service_name_ptr = iox2_cast_service_name_ptr(service_name);
    iox2_service_builder_h service_builder = iox2_node_service_builder(&node, NULL, service_name_ptr);
    iox2_service_builder_pub_sub_h service_builder_pub_sub = iox2_service_builder_pub_sub(service_builder);

    const char* payload_type_name = "Distance";
    if (iox2_service_builder_pub_sub_set_payload_type_details(&service_builder_pub_sub,
                                                              iox2_type_variant_e_FIXED_SIZE,
                                                              payload_type_name,
                                                              strlen(payload_type_name),
                                                              sizeof(struct Distance),
                                                              alignof(struct Distance))
        != IOX2_OK) {
        printf("Unable to set type details\n");
        exit(-1);
    }

    iox2_service_builder_pub_sub_set_subscriber_max_buffer_size(&service_builder_pub_sub, 3);
    iox2_service_builder_pub_sub_set_history_size(&service_builder_pub_sub, 3);
    iox2_service_builder_pub_sub_set_subscriber_max_borrowed_samples(&service_builder_pub_sub, 3);

    iox2_port_factory_pub_sub_h pubsub_service = NULL;
    if (iox2_service_builder_pub_sub_open_or_create(service_builder_pub_sub, NULL, &pubsub_service) != IOX2_OK) {
        printf("Unable to create service!\n");
        exit(-1);
    }

    iox2_service_builder_event_h service_builder_event =
        iox2_service_builder_event(iox2_node_service_builder(&node, NULL, iox2_cast_service_name_ptr(service_name)));
    iox2_service_builder_event_set_notifier_dead_event(&service_builder_event, ultra_sonic_service_dead.value);

    iox2_port_factory_event_h event_service = NULL;
    if (iox2_service_builder_event_open_or_create(service_builder_event, NULL, &event_service) != IOX2_OK) {
        printf("Unable to create event service!\n");
        exit(-1);
    }

    iox2_port_factory_subscriber_builder_h subscriber_builder =
        iox2_port_factory_pub_sub_subscriber_builder(&pubsub_service, NULL);
    iox2_subscriber_h subscriber = NULL;
    if (iox2_port_factory_subscriber_builder_create(subscriber_builder, NULL, &subscriber) != IOX2_OK) {
        printf("Unable to create subscriber!\n");
        exit(-1);
    }

    iox2_port_factory_listener_builder_h listener_builder =
        iox2_port_factory_event_listener_builder(&event_service, NULL);
    iox2_listener_h listener = NULL;
    if (iox2_port_factory_listener_builder_create(listener_builder, NULL, &listener) != IOX2_OK) {
        printf("Unable to create listener!\n");
        exit(-1);
    }
    // snippet:end setup

    // snippet:start wait-loop
    struct received_events events;
    iox2_sample_h last_samples[3];
    uint64_t number_of_notifications = 0;
    while (iox2_node_wait(&node, 0, 0) == IOX2_OK) {
        events.sensor_is_dead = false;
        events.obstacle_detected = false;

        // C has no closures, so the wait callback must be a separate static
        // function rather than being written inline. `collect_event` just
        // records which events fired into `events`:
        //
        //   static void collect_event(const iox2_event_id_t* id, uint64_t count, void* ctx) {
        //       struct received_events* events = ctx;
        //       events->sensor_is_dead |= id->value == ultra_sonic_service_dead.value;
        //       events->obstacle_detected |= id->value == obstacle_too_close.value;
        //   }
        if (iox2_listener_blocking_wait(&listener, &number_of_notifications, collect_event, &events) != IOX2_OK) {
            printf("Unable to wait for notification!\n");
            exit(-1);
        }

        if (events.sensor_is_dead) {
            go_into_parking_position();
        }

        if (events.obstacle_detected) {
            size_t received = 0;
            for (size_t i = 0; i < 3; ++i) {
                if (iox2_subscriber_receive(&subscriber, NULL, &last_samples[i]) != IOX2_OK) {
                    printf("Failed to receive sample\n");
                    exit(-1);
                }
                if (last_samples[i] == NULL) {
                    break;
                }
                received++;
            }

            perform_break(last_samples, received);

            for (size_t i = 0; i < received; ++i) {
                iox2_sample_drop(last_samples[i]);
            }
        }
    }

    // release every handle once the loop exits
    iox2_listener_drop(listener);
    iox2_subscriber_drop(subscriber);
    iox2_port_factory_event_drop(event_service);
    iox2_port_factory_pub_sub_drop(pubsub_service);
    iox2_service_name_drop(service_name);
    iox2_node_drop(node);
    // snippet:end wait-loop

    return 0;
}
