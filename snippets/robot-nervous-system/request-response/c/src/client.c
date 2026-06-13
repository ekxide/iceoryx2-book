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

// stand-ins for the user-facing app
static void show_larry_position_in_app(const struct State* state) {
    (void) state;
}

static bool is_at_destination(const struct State* state) {
    (void) state;
    return false;
}

static void show_larry_arrived_popup_in_app(void) {
}

static void show_larry_encountered_obstacle_in_app(void) {
}

static bool user_has_pressed_stop_button(void) {
    return false;
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
    // snippet:end service

    // snippet:start client
    iox2_port_factory_client_builder_h client_builder =
        iox2_port_factory_request_response_client_builder(&service, NULL);
    iox2_client_h client = NULL;
    if (iox2_port_factory_client_builder_create(client_builder, NULL, &client) != IOX2_OK) {
        printf("Unable to create client!\n");
        exit(-1);
    }
    // snippet:end client

    // snippet:start send-request
    iox2_request_mut_h request = NULL;
    if (iox2_client_loan_slice_uninit(&client, NULL, &request, 1) != IOX2_OK) {
        printf("Failed to loan request\n");
        exit(-1);
    }

    struct Position* payload = NULL;
    iox2_request_mut_payload_mut(&request, (void**) &payload, NULL);
    payload->position[0] = 123.456F; // NOLINT
    payload->position[1] = 789.1F;   // NOLINT

    iox2_pending_response_h pending_response = NULL;
    if (iox2_request_mut_send(request, NULL, &pending_response) != IOX2_OK) {
        printf("Failed to send request\n");
        exit(-1);
    }
    // snippet:end send-request

    // snippet:start event-loop
    while (iox2_node_wait(&node, 0, 100000000) == IOX2_OK) {
        iox2_response_h response = NULL;
        while (true) {
            response = NULL;
            if (iox2_pending_response_receive(&pending_response, NULL, &response) != IOX2_OK) {
                printf("Failed to receive response\n");
                exit(-1);
            }
            if (response == NULL) {
                break;
            }

            const struct State* state = NULL;
            iox2_response_payload(&response, (const void**) &state, NULL);
            show_larry_position_in_app(state);

            if (!iox2_pending_response_is_connected(&pending_response)) {
                if (is_at_destination(state)) {
                    show_larry_arrived_popup_in_app();
                } else {
                    show_larry_encountered_obstacle_in_app();
                }
            }

            iox2_response_drop(response);
        }

        if (user_has_pressed_stop_button()) {
            // dropping the pending response cancels the stream
            break;
        }
    }

    // release every handle once the loop exits
    iox2_pending_response_drop(pending_response);
    iox2_client_drop(client);
    iox2_port_factory_request_response_drop(service);
    iox2_service_name_drop(service_name);
    iox2_node_drop(node);
    // snippet:end event-loop

    return 0;
}
