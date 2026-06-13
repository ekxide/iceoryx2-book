#include "distance.h"
#include "iox2/iceoryx2.h"

#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__) || defined(_WIN64)
#define alignof __alignof
#else
#include <stdalign.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// stands in for the platform-specific ultrasonic sensor read
static double get_ultra_sonic_sensor_distance(void) {
    return 0.42; // NOLINT
}

int main(void) {
    iox2_node_builder_h node_builder = iox2_node_builder_new(NULL);
    iox2_node_h node = NULL;
    if (iox2_node_builder_create(node_builder, NULL, iox2_service_type_e_IPC, &node) != IOX2_OK) {
        printf("Could not create node!\n");
        exit(-1);
    }

    // the distance publisher is reused from the publish-subscribe example
    const char* distance_service_name_value = "distance_to_obstacle";
    iox2_service_name_h distance_service_name = NULL;
    iox2_service_name_new(
        NULL, distance_service_name_value, strlen(distance_service_name_value), &distance_service_name);
    iox2_service_builder_pub_sub_h distance_builder = iox2_service_builder_pub_sub(
        iox2_node_service_builder(&node, NULL, iox2_cast_service_name_ptr(distance_service_name)));
    const char* distance_type_name = "Distance";
    iox2_service_builder_pub_sub_set_payload_type_details(&distance_builder,
                                                          iox2_type_variant_e_FIXED_SIZE,
                                                          distance_type_name,
                                                          strlen(distance_type_name),
                                                          sizeof(struct Distance),
                                                          alignof(struct Distance));
    iox2_port_factory_pub_sub_h distance_service = NULL;
    if (iox2_service_builder_pub_sub_open_or_create(distance_builder, NULL, &distance_service) != IOX2_OK) {
        printf("Unable to create distance service!\n");
        exit(-1);
    }
    iox2_publisher_h publisher = NULL;
    iox2_port_factory_publisher_builder_create(
        iox2_port_factory_pub_sub_publisher_builder(&distance_service, NULL), NULL, &publisher);

    // snippet:start service
    const char* service_name_value = "global_config";
    iox2_service_name_h service_name = NULL;
    if (iox2_service_name_new(NULL, service_name_value, strlen(service_name_value), &service_name) != IOX2_OK) {
        printf("Unable to create service name!\n");
        exit(-1);
    }

    iox2_service_name_ptr service_name_ptr = iox2_cast_service_name_ptr(service_name);
    iox2_service_builder_h service_builder = iox2_node_service_builder(&node, NULL, service_name_ptr);
    iox2_service_builder_blackboard_opener_h service_builder_blackboard =
        iox2_service_builder_blackboard_opener(service_builder);

    const char* key_type_name = "uint64_t";
    if (iox2_service_builder_blackboard_opener_set_key_type_details(
            &service_builder_blackboard, key_type_name, strlen(key_type_name), sizeof(uint64_t), alignof(uint64_t))
        != IOX2_OK) {
        printf("Unable to set key type details!\n");
        exit(-1);
    }

    iox2_port_factory_blackboard_h service = NULL;
    if (iox2_service_builder_blackboard_open(service_builder_blackboard, NULL, &service) != IOX2_OK) {
        printf("Unable to open service!\n");
        exit(-1);
    }
    // snippet:end service

    // snippet:start reader
    iox2_port_factory_reader_builder_h reader_builder = iox2_port_factory_blackboard_reader_builder(&service, NULL);
    iox2_reader_h reader = NULL;
    if (iox2_port_factory_reader_builder_create(reader_builder, NULL, &reader) != IOX2_OK) {
        printf("Unable to create reader!\n");
        exit(-1);
    }
    // snippet:end reader

    // snippet:start handle
    uint64_t us_sensor_key = 1;
    const char* us_sensor_value_type_name = "uint32_t";
    iox2_entry_handle_h update_rate_handle = NULL;
    if (iox2_reader_entry(&reader,
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
    // snippet:end handle

    // snippet:start sensor-loop
    uint32_t new_update_rate = 0;
    iox2_entry_handle_get(&update_rate_handle, &new_update_rate, sizeof(uint32_t), alignof(uint32_t), NULL);
    while (iox2_node_wait(&node, 0, new_update_rate * 1000000) == IOX2_OK) {
        iox2_sample_mut_h sample = NULL;
        if (iox2_publisher_loan_slice_uninit(&publisher, NULL, &sample, 1) != IOX2_OK) {
            printf("Failed to loan sample\n");
            exit(-1);
        }

        struct Distance* payload = NULL;
        iox2_sample_mut_payload_mut(&sample, (void**) &payload, NULL);
        payload->distance_in_meters = get_ultra_sonic_sensor_distance();
        payload->some_other_property = 42.0F; // NOLINT

        if (iox2_sample_mut_send(sample, NULL) != IOX2_OK) {
            printf("Failed to send sample\n");
            exit(-1);
        }

        iox2_entry_handle_get(&update_rate_handle, &new_update_rate, sizeof(uint32_t), alignof(uint32_t), NULL);
    }

    // release every handle once the loop exits
    iox2_entry_handle_drop(update_rate_handle);
    iox2_reader_drop(reader);
    iox2_port_factory_blackboard_drop(service);
    iox2_service_name_drop(service_name);
    iox2_publisher_drop(publisher);
    iox2_port_factory_pub_sub_drop(distance_service);
    iox2_service_name_drop(distance_service_name);
    iox2_node_drop(node);
    // snippet:end sensor-loop

    return 0;
}
