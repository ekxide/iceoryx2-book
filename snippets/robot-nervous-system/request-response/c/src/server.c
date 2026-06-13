#include "autopilot.h"
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

// stand-ins for Larry's autopilot
static void drive_to_position(const struct Position* request) {
    (void) request;
}

static void stop_driving(void) {
}

static bool arrived_at_destination(void) {
    return false;
}

static void get_current_state(struct State* state) {
    state->position[0] = 0.0F;
    state->position[1] = 0.0F;
    state->speed[0] = 0.0F;
    state->speed[1] = 0.0F;
}

int main(void) {
    // snippet:start setup
    iox2_node_builder_h node_builder = iox2_node_builder_new(NULL);
    iox2_node_h node = NULL;
    if (iox2_node_builder_create(node_builder, NULL, iox2_service_type_e_IPC, &node) != IOX2_OK) {
        printf("Could not create node!\n");
        exit(-1);
    }

    const char* service_name_value = "autopilot";
    iox2_service_name_h service_name = NULL;
    if (iox2_service_name_new(NULL, service_name_value, strlen(service_name_value), &service_name) != IOX2_OK) {
        printf("Unable to create service name!\n");
        exit(-1);
    }

    iox2_service_name_ptr service_name_ptr = iox2_cast_service_name_ptr(service_name);
    iox2_service_builder_h service_builder = iox2_node_service_builder(&node, NULL, service_name_ptr);
    iox2_service_builder_request_response_h service_builder_request_response =
        iox2_service_builder_request_response(service_builder);

    const char* request_type_name = "Position";
    if (iox2_service_builder_request_response_set_request_payload_type_details(&service_builder_request_response,
                                                                               iox2_type_variant_e_FIXED_SIZE,
                                                                               request_type_name,
                                                                               strlen(request_type_name),
                                                                               sizeof(struct Position),
                                                                               alignof(struct Position))
        != IOX2_OK) {
        printf("Unable to set request type details\n");
        exit(-1);
    }

    const char* response_type_name = "State";
    if (iox2_service_builder_request_response_set_response_payload_type_details(&service_builder_request_response,
                                                                                iox2_type_variant_e_FIXED_SIZE,
                                                                                response_type_name,
                                                                                strlen(response_type_name),
                                                                                sizeof(struct State),
                                                                                alignof(struct State))
        != IOX2_OK) {
        printf("Unable to set response type details\n");
        exit(-1);
    }

    iox2_port_factory_request_response_h service = NULL;
    if (iox2_service_builder_request_response_open_or_create(service_builder_request_response, NULL, &service)
        != IOX2_OK) {
        printf("Unable to create service!\n");
        exit(-1);
    }

    iox2_port_factory_server_builder_h server_builder =
        iox2_port_factory_request_response_server_builder(&service, NULL);
    iox2_server_h server = NULL;
    if (iox2_port_factory_server_builder_create(server_builder, NULL, &server) != IOX2_OK) {
        printf("Unable to create server!\n");
        exit(-1);
    }
    // snippet:end setup

    // snippet:start serve-loop
    iox2_active_request_h active_request = NULL;
    while (iox2_node_wait(&node, 0, 100000000) == IOX2_OK) {
        if (active_request == NULL) {
            if (iox2_server_receive(&server, NULL, &active_request) != IOX2_OK) {
                printf("Failed to receive request\n");
                exit(-1);
            }

            if (active_request != NULL) {
                const struct Position* position = NULL;
                iox2_active_request_payload(&active_request, (const void**) &position, NULL);
                drive_to_position(position);
            }
        }

        if (active_request != NULL) {
            if (!iox2_active_request_is_connected(&active_request)) {
                stop_driving();
                iox2_active_request_drop(active_request);
                active_request = NULL;
            } else if (arrived_at_destination()) {
                iox2_active_request_drop(active_request);
                active_request = NULL;
            } else {
                iox2_response_mut_h response = NULL;
                if (iox2_active_request_loan_slice_uninit(&active_request, NULL, &response, 1) != IOX2_OK) {
                    printf("Failed to loan response\n");
                    exit(-1);
                }

                struct State* state = NULL;
                iox2_response_mut_payload_mut(&response, (void**) &state, NULL);
                get_current_state(state);

                if (iox2_response_mut_send(response) != IOX2_OK) {
                    printf("Failed to send response\n");
                    exit(-1);
                }
            }
        }
    }

    // release every handle once the loop exits
    iox2_server_drop(server);
    iox2_port_factory_request_response_drop(service);
    iox2_service_name_drop(service_name);
    iox2_node_drop(node);
    // snippet:end serve-loop

    return 0;
}
