#include "iox2/iceoryx2.h"
#include "telemetry_data.h"

#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__) || defined(_WIN64)
#define alignof __alignof
#else
#include <stdalign.h>
#endif
#include <stdio.h>
#include <string.h>

// snippet:start
#define CYCLE_TIME_SECS 1
#define CYCLE_TIME_NANOS 0

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

    // create publishers
    iox2_publisher_h battery_publisher = NULL;
    iox2_port_factory_publisher_builder_create(
        iox2_port_factory_pub_sub_publisher_builder(&battery_service, NULL), NULL, &battery_publisher);
    iox2_publisher_h position_publisher = NULL;
    iox2_port_factory_publisher_builder_create(
        iox2_port_factory_pub_sub_publisher_builder(&position_service, NULL), NULL, &position_publisher);

    // create notifiers
    iox2_notifier_h battery_notifier = NULL;
    iox2_port_factory_notifier_builder_create(
        iox2_port_factory_event_notifier_builder(&battery_event_service, NULL), NULL, &battery_notifier);
    iox2_notifier_h position_notifier = NULL;
    iox2_port_factory_notifier_builder_create(
        iox2_port_factory_event_notifier_builder(&position_event_service, NULL), NULL, &position_notifier);

    while (iox2_node_wait(&node, CYCLE_TIME_SECS, CYCLE_TIME_NANOS) == IOX2_OK) {
        // publish battery state and notify
        iox2_sample_mut_h battery_sample = NULL;
        if (iox2_publisher_loan_slice_uninit(&battery_publisher, NULL, &battery_sample, 1) == IOX2_OK) {
            struct BatteryState* battery_payload = NULL;
            iox2_sample_mut_payload_mut(&battery_sample, (void**) &battery_payload, NULL);
            battery_payload->charge_percent = 87.0F; // NOLINT
            iox2_sample_mut_send(battery_sample, NULL);
            iox2_notifier_notify(&battery_notifier, NULL);
        }

        // publish position and notify
        iox2_sample_mut_h position_sample = NULL;
        if (iox2_publisher_loan_slice_uninit(&position_publisher, NULL, &position_sample, 1) == IOX2_OK) {
            struct Position* position_payload = NULL;
            iox2_sample_mut_payload_mut(&position_sample, (void**) &position_payload, NULL);
            position_payload->x = 1.0F; // NOLINT
            position_payload->y = 2.0F; // NOLINT
            iox2_sample_mut_send(position_sample, NULL);
            iox2_notifier_notify(&position_notifier, NULL);
        }
    }

    iox2_notifier_drop(position_notifier);
    iox2_notifier_drop(battery_notifier);
    iox2_publisher_drop(position_publisher);
    iox2_publisher_drop(battery_publisher);
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
// snippet:end
