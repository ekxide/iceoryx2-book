#include "iox2/iceoryx2.h"
#include "telemetry_data.h"

#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__) || defined(_WIN64)
#define alignof __alignof
#else
#include <stdalign.h>
#endif
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// snippet:start
struct CallbackContext {
    iox2_waitset_guard_h_ref battery_guard;
    iox2_waitset_guard_h_ref position_guard;
    iox2_listener_h_ref battery_listener;
    iox2_listener_h_ref position_listener;
    iox2_subscriber_h_ref battery_subscriber;
    iox2_subscriber_h_ref position_subscriber;
};

// no-op callback: draining only needs to clear the queue, not inspect events
static void drain_event(const iox2_event_id_t* event_id, uint64_t count, void* context) {
    (void) event_id;
    (void) count;
    (void) context;
}

// called whenever a listener attached to the WaitSet has received an event
static iox2_callback_progression_e on_event(iox2_waitset_attachment_id_h attachment_id, void* context) {
    struct CallbackContext* ctx = (struct CallbackContext*) context;

    uint64_t number_of_notifications = 0;

    if (iox2_waitset_attachment_id_has_event_from(&attachment_id, ctx->battery_guard)) {
        // drain every pending notification, otherwise the WaitSet wakes
        // again immediately and spins
        iox2_listener_try_wait(ctx->battery_listener, &number_of_notifications, drain_event, NULL);

        iox2_sample_h battery_sample = NULL;
        while (iox2_subscriber_receive(ctx->battery_subscriber, NULL, &battery_sample) == IOX2_OK
               && battery_sample != NULL) {
            struct BatteryState* payload = NULL;
            iox2_sample_payload(&battery_sample, (const void**) &payload, NULL);
            printf("battery: %.1f%%\n", (double) payload->charge_percent);
            iox2_sample_drop(battery_sample);
            battery_sample = NULL;
        }
    } else if (iox2_waitset_attachment_id_has_event_from(&attachment_id, ctx->position_guard)) {
        // drain every pending notification, otherwise the WaitSet wakes
        // again immediately and spins
        iox2_listener_try_wait(ctx->position_listener, &number_of_notifications, drain_event, NULL);

        iox2_sample_h position_sample = NULL;
        while (iox2_subscriber_receive(ctx->position_subscriber, NULL, &position_sample) == IOX2_OK
               && position_sample != NULL) {
            struct Position* payload = NULL;
            iox2_sample_payload(&position_sample, (const void**) &payload, NULL);
            printf("position: (%.1f, %.1f)\n", (double) payload->x, (double) payload->y);
            iox2_sample_drop(position_sample);
            position_sample = NULL;
        }
    }

    iox2_waitset_attachment_id_drop(attachment_id);
    return iox2_callback_progression_e_CONTINUE;
}

// NOLINTBEGIN(readability-function-size)
int main(void) {
    iox2_set_log_level_from_env_or(iox2_log_level_e_INFO);
    int ret_val = 0;

    // create node
    iox2_node_builder_h node_builder = iox2_node_builder_new(NULL);
    iox2_node_h node = NULL;
    ret_val = iox2_node_builder_create(node_builder, NULL, iox2_service_type_e_IPC, &node);
    if (ret_val != IOX2_OK) {
        printf("Could not create node! Error: %d\n", ret_val);
        goto end;
    }

    // create battery publish-subscribe service
    iox2_service_name_h battery_name = NULL;
    const char* battery_name_value = "larry/battery";
    iox2_service_name_new(NULL, battery_name_value, strlen(battery_name_value), &battery_name);
    iox2_service_builder_pub_sub_h battery_builder =
        iox2_service_builder_pub_sub(iox2_node_service_builder(&node, NULL, iox2_cast_service_name_ptr(battery_name)));
    iox2_service_builder_pub_sub_set_payload_type_details(&battery_builder,
                                                          iox2_type_variant_e_FIXED_SIZE,
                                                          BATTERY_STATE_TYPE_NAME,
                                                          strlen(BATTERY_STATE_TYPE_NAME),
                                                          sizeof(struct BatteryState),
                                                          alignof(struct BatteryState));
    iox2_port_factory_pub_sub_h battery_service = NULL;
    ret_val = iox2_service_builder_pub_sub_open_or_create(battery_builder, NULL, &battery_service);
    if (ret_val != IOX2_OK) {
        printf("Could not create battery service! Error: %d\n", ret_val);
        goto drop_node;
    }

    // create position publish-subscribe service
    iox2_service_name_h position_name = NULL;
    const char* position_name_value = "larry/position";
    iox2_service_name_new(NULL, position_name_value, strlen(position_name_value), &position_name);
    iox2_service_builder_pub_sub_h position_builder =
        iox2_service_builder_pub_sub(iox2_node_service_builder(&node, NULL, iox2_cast_service_name_ptr(position_name)));
    iox2_service_builder_pub_sub_set_payload_type_details(&position_builder,
                                                          iox2_type_variant_e_FIXED_SIZE,
                                                          POSITION_TYPE_NAME,
                                                          strlen(POSITION_TYPE_NAME),
                                                          sizeof(struct Position),
                                                          alignof(struct Position));
    iox2_port_factory_pub_sub_h position_service = NULL;
    ret_val = iox2_service_builder_pub_sub_open_or_create(position_builder, NULL, &position_service);
    if (ret_val != IOX2_OK) {
        printf("Could not create position service! Error: %d\n", ret_val);
        goto drop_battery_service;
    }

    // create battery event service
    iox2_service_builder_event_h battery_event_builder =
        iox2_service_builder_event(iox2_node_service_builder(&node, NULL, iox2_cast_service_name_ptr(battery_name)));
    iox2_port_factory_event_h battery_event_service = NULL;
    ret_val = iox2_service_builder_event_open_or_create(battery_event_builder, NULL, &battery_event_service);
    if (ret_val != IOX2_OK) {
        printf("Could not create battery event service! Error: %d\n", ret_val);
        goto drop_position_service;
    }

    // create position event service
    iox2_service_builder_event_h position_event_builder =
        iox2_service_builder_event(iox2_node_service_builder(&node, NULL, iox2_cast_service_name_ptr(position_name)));
    iox2_port_factory_event_h position_event_service = NULL;
    ret_val = iox2_service_builder_event_open_or_create(position_event_builder, NULL, &position_event_service);
    if (ret_val != IOX2_OK) {
        printf("Could not create position event service! Error: %d\n", ret_val);
        goto drop_battery_event_service;
    }

    // create subscribers
    iox2_subscriber_h battery_subscriber = NULL;
    iox2_port_factory_subscriber_builder_create(
        iox2_port_factory_pub_sub_subscriber_builder(&battery_service, NULL), NULL, &battery_subscriber);
    iox2_subscriber_h position_subscriber = NULL;
    iox2_port_factory_subscriber_builder_create(
        iox2_port_factory_pub_sub_subscriber_builder(&position_service, NULL), NULL, &position_subscriber);

    // create listeners
    iox2_listener_h battery_listener = NULL;
    iox2_port_factory_listener_builder_create(
        iox2_port_factory_event_listener_builder(&battery_event_service, NULL), NULL, &battery_listener);
    iox2_listener_h position_listener = NULL;
    iox2_port_factory_listener_builder_create(
        iox2_port_factory_event_listener_builder(&position_event_service, NULL), NULL, &position_listener);

    // create waitset and attach both listeners
    iox2_waitset_builder_h waitset_builder = NULL;
    iox2_waitset_builder_new(NULL, &waitset_builder);
    iox2_waitset_h waitset = NULL;
    ret_val = iox2_waitset_builder_create(waitset_builder, iox2_service_type_e_IPC, NULL, &waitset);
    if (ret_val != IOX2_OK) {
        printf("Could not create waitset! Error: %d\n", ret_val);
        goto drop_listeners;
    }

    iox2_waitset_guard_h battery_guard = NULL;
    ret_val = iox2_waitset_attach_notification(
        &waitset, iox2_listener_get_file_descriptor(&battery_listener), NULL, &battery_guard);
    if (ret_val != IOX2_OK) {
        printf("Could not attach battery listener! Error: %d\n", ret_val);
        goto drop_waitset;
    }

    iox2_waitset_guard_h position_guard = NULL;
    ret_val = iox2_waitset_attach_notification(
        &waitset, iox2_listener_get_file_descriptor(&position_listener), NULL, &position_guard);
    if (ret_val != IOX2_OK) {
        printf("Could not attach position listener! Error: %d\n", ret_val);
        goto drop_battery_guard;
    }

    struct CallbackContext context;
    context.battery_guard = &battery_guard;
    context.position_guard = &position_guard;
    context.battery_listener = &battery_listener;
    context.position_listener = &position_listener;
    context.battery_subscriber = &battery_subscriber;
    context.position_subscriber = &position_subscriber;

    iox2_waitset_run_result_e result = iox2_waitset_run_result_e_STOP_REQUEST;
    ret_val = iox2_waitset_wait_and_process(&waitset, on_event, (void*) &context, &result);
    if (ret_val != IOX2_OK) {
        printf("Failure in WaitSet wait_and_process loop! Error: %d\n", ret_val);
    }

    iox2_waitset_guard_drop(position_guard);
drop_battery_guard:
    iox2_waitset_guard_drop(battery_guard);
drop_waitset:
    iox2_waitset_drop(waitset);
drop_listeners:
    iox2_listener_drop(position_listener);
    iox2_listener_drop(battery_listener);
    iox2_subscriber_drop(position_subscriber);
    iox2_subscriber_drop(battery_subscriber);
    iox2_port_factory_event_drop(position_event_service);
drop_battery_event_service:
    iox2_port_factory_event_drop(battery_event_service);
drop_position_service:
    iox2_port_factory_pub_sub_drop(position_service);
drop_battery_service:
    iox2_port_factory_pub_sub_drop(battery_service);
drop_node:
    iox2_node_drop(node);
end:
    return ret_val;
}
// NOLINTEND(readability-function-size)
//  snippet:end
