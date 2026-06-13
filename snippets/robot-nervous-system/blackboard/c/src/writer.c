#include "iox2/iceoryx2.h"

#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__) || defined(_WIN64)
#define alignof __alignof
#else
#include <stdalign.h>
#endif
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// stand-ins for the user-facing configuration input
static bool get_battery_threshold(float* new_battery_threshold) {
    (void) new_battery_threshold;
    return false;
}

static bool get_update_rate(uint32_t* new_update_rate) {
    (void) new_update_rate;
    return false;
}

// keys are compared by value
static bool key_cmp(const void* lhs, const void* rhs) {
    return *((const uint64_t*) lhs) == *((const uint64_t*) rhs);
}

int main(void) {
    // snippet:start node
    iox2_node_builder_h node_builder = iox2_node_builder_new(NULL);
    iox2_node_h node = NULL;
    if (iox2_node_builder_create(node_builder, NULL, iox2_service_type_e_IPC, &node) != IOX2_OK) {
        printf("Could not create node!\n");
        exit(-1);
    }
    // snippet:end node

    // snippet:start service
    const char* service_name_value = "global_config";
    iox2_service_name_h service_name = NULL;
    if (iox2_service_name_new(NULL, service_name_value, strlen(service_name_value), &service_name) != IOX2_OK) {
        printf("Unable to create service name!\n");
        exit(-1);
    }

    iox2_service_name_ptr service_name_ptr = iox2_cast_service_name_ptr(service_name);
    iox2_service_builder_h service_builder = iox2_node_service_builder(&node, NULL, service_name_ptr);
    iox2_service_builder_blackboard_creator_h service_builder_blackboard =
        iox2_service_builder_blackboard_creator(service_builder);

    const char* key_type_name = "uint64_t";
    if (iox2_service_builder_blackboard_creator_set_key_type_details(
            &service_builder_blackboard, key_type_name, strlen(key_type_name), sizeof(uint64_t), alignof(uint64_t))
        != IOX2_OK) {
        printf("Unable to set key type details!\n");
        exit(-1);
    }

    iox2_service_builder_blackboard_creator_set_key_eq_comparison_function(&service_builder_blackboard, key_cmp);

    uint64_t battery_key = 0;
    const char* battery_value_type_name = "float";
    float battery_value = 0.25F; // NOLINT
    iox2_service_builder_blackboard_creator_add(&service_builder_blackboard,
                                                &battery_key,
                                                &battery_value,
                                                NULL,
                                                battery_value_type_name,
                                                strlen(battery_value_type_name),
                                                sizeof(float),
                                                alignof(float));

    uint64_t us_sensor_key = 1;
    const char* us_sensor_value_type_name = "uint32_t";
    uint32_t us_sensor_value = 100;
    iox2_service_builder_blackboard_creator_add(&service_builder_blackboard,
                                                &us_sensor_key,
                                                &us_sensor_value,
                                                NULL,
                                                us_sensor_value_type_name,
                                                strlen(us_sensor_value_type_name),
                                                sizeof(uint32_t),
                                                alignof(uint32_t));

    iox2_port_factory_blackboard_h service = NULL;
    if (iox2_service_builder_blackboard_create(service_builder_blackboard, NULL, &service) != IOX2_OK) {
        printf("Unable to create service!\n");
        exit(-1);
    }
    // snippet:end service

    // snippet:start writer
    iox2_port_factory_writer_builder_h writer_builder = iox2_port_factory_blackboard_writer_builder(&service, NULL);
    iox2_writer_h writer = NULL;
    if (iox2_port_factory_writer_builder_create(writer_builder, NULL, &writer) != IOX2_OK) {
        printf("Unable to create writer!\n");
        exit(-1);
    }
    // snippet:end writer

    // snippet:start entries
    iox2_entry_handle_mut_h battery_threshold_handle = NULL;
    if (iox2_writer_entry(&writer,
                          NULL,
                          &battery_threshold_handle,
                          &battery_key,
                          battery_value_type_name,
                          strlen(battery_value_type_name),
                          sizeof(float),
                          alignof(float))
        != IOX2_OK) {
        printf("Unable to create battery threshold handle!\n");
        exit(-1);
    }

    iox2_entry_handle_mut_h update_rate_handle = NULL;
    if (iox2_writer_entry(&writer,
                          NULL,
                          &update_rate_handle,
                          &us_sensor_key,
                          us_sensor_value_type_name,
                          strlen(us_sensor_value_type_name),
                          sizeof(uint32_t),
                          alignof(uint32_t))
        != IOX2_OK) {
        printf("Unable to create update rate handle!\n");
        exit(-1);
    }
    // snippet:end entries

    // snippet:start update-loop
    while (iox2_node_wait(&node, 0, 100000000) == IOX2_OK) {
        float new_battery_threshold = 0.0F;
        if (get_battery_threshold(&new_battery_threshold)) {
            // small value -> simple copy API
            iox2_entry_handle_mut_update_with_copy(
                &battery_threshold_handle, &new_battery_threshold, sizeof(float), alignof(float));
        }

        uint32_t new_update_rate = 0;
        if (get_update_rate(&new_update_rate)) {
            iox2_entry_handle_mut_update_with_copy(
                &update_rate_handle, &new_update_rate, sizeof(uint32_t), alignof(uint32_t));
        }
    }

    // release every handle once the loop exits
    iox2_entry_handle_mut_drop(update_rate_handle);
    iox2_entry_handle_mut_drop(battery_threshold_handle);
    iox2_writer_drop(writer);
    iox2_port_factory_blackboard_drop(service);
    iox2_service_name_drop(service_name);
    iox2_node_drop(node);
    // snippet:end update-loop

    return 0;
}
