#include "distance.h"
#include "iox2/iceoryx2.h"

#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__) || defined(_WIN64)
#define alignof __alignof
#else
#include <stdalign.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// stands in for the platform-specific ultrasonic sensor read
static double get_ultra_sonic_sensor_distance(void) {
    return 0.42; // NOLINT
}

int main(void) {
    // snippet:start node
    iox2_node_builder_h node_builder = iox2_node_builder_new(NULL);

    const char* node_name_value = "UltraSonicSensor";
    iox2_node_name_h node_name = NULL;
    if (iox2_node_name_new(NULL, node_name_value, strlen(node_name_value), &node_name) != IOX2_OK) {
        printf("Unable to set node name!\n");
        exit(-1);
    }

    iox2_node_name_ptr node_name_ptr = iox2_cast_node_name_ptr(node_name);
    iox2_node_builder_set_name(&node_builder, node_name_ptr);

    iox2_node_h node = NULL;
    if (iox2_node_builder_create(node_builder, NULL, iox2_service_type_e_IPC, &node) != IOX2_OK) {
        printf("Could not create node!\n");
        exit(-1);
    }
    // snippet:end node

    // snippet:start service
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

    iox2_port_factory_pub_sub_h service = NULL;
    if (iox2_service_builder_pub_sub_open_or_create(service_builder_pub_sub, NULL, &service) != IOX2_OK) {
        printf("Unable to create service!\n");
        exit(-1);
    }
    // snippet:end service

    // snippet:start publisher
    iox2_port_factory_publisher_builder_h publisher_builder =
        iox2_port_factory_pub_sub_publisher_builder(&service, NULL);
    iox2_publisher_h publisher = NULL;
    if (iox2_port_factory_publisher_builder_create(publisher_builder, NULL, &publisher) != IOX2_OK) {
        printf("Unable to create publisher!\n");
        exit(-1);
    }
    // snippet:end publisher

    // snippet:start publish-loop
    while (iox2_node_wait(&node, 0, 10000000) == IOX2_OK) {
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
    }

    // release every handle once the loop exits
    iox2_publisher_drop(publisher);
    iox2_port_factory_pub_sub_drop(service);
    iox2_service_name_drop(service_name);
    iox2_node_name_drop(node_name);
    iox2_node_drop(node);
    // snippet:end publish-loop

    return 0;
}
