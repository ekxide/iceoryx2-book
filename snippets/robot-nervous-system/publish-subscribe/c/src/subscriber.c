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

int main(void) {
    // snippet:start node-and-service
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

    iox2_port_factory_pub_sub_h service = NULL;
    if (iox2_service_builder_pub_sub_open_or_create(service_builder_pub_sub, NULL, &service) != IOX2_OK) {
        printf("Unable to create service!\n");
        exit(-1);
    }
    // snippet:end node-and-service

    // snippet:start subscriber
    iox2_port_factory_subscriber_builder_h subscriber_builder =
        iox2_port_factory_pub_sub_subscriber_builder(&service, NULL);
    iox2_subscriber_h subscriber = NULL;
    if (iox2_port_factory_subscriber_builder_create(subscriber_builder, NULL, &subscriber) != IOX2_OK) {
        printf("Unable to create subscriber!\n");
        exit(-1);
    }
    // snippet:end subscriber

    // snippet:start receive-loop
    while (iox2_node_wait(&node, 0, 10000000) == IOX2_OK) {
        iox2_sample_h sample = NULL;
        if (iox2_subscriber_receive(&subscriber, NULL, &sample) != IOX2_OK) {
            printf("Failed to receive sample\n");
            exit(-1);
        }

        if (sample != NULL) {
            struct Distance* payload = NULL;
            iox2_sample_payload(&sample, (const void**) &payload, NULL);

            printf("received: Distance { .distance_in_meters: %f, .some_other_property: %f }\n",
                   payload->distance_in_meters,
                   (double) payload->some_other_property);
            iox2_sample_drop(sample);
        }
    }

    // release every handle once the loop exits
    iox2_subscriber_drop(subscriber);
    iox2_port_factory_pub_sub_drop(service);
    iox2_service_name_drop(service_name);
    iox2_node_drop(node);
    // snippet:end receive-loop

    return 0;
}
