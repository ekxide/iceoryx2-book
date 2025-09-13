# Request-Response

With emergency brakes and some basic fault handling in place, it’s time for
something a bit more sophisticated.

Imagine the user wants to send our robot Larry to a specific position:
“Go to point B.”
As Larry starts driving, the user expects status updates - position, speed,
progress. At some point the user may even cancel the trip, or Larry himself may
have to abort if he encounters, say, a canyon.

```{figure} /images/robot-nervous-system-request-response-sequence.svg
:alt: Request-Response Sequence Diagram between App and Larry Autopilot
:align: center
:name: fig-wiring-a-robot-brain-request-response-sequence

Request-Response interactions between App and Larry Autopilot
```

This is where the request-response (stream) pattern in `iceoryx2` comes into
play. It behaves like regular non-blocking request-response, but with one extra
trick: the server can send a stream of responses instead of just one. Either
side can end the stream at any time if it’s no longer interested.

## Client

The client is our user-facing app. The request can be a simple 2D position:

````{tab-set-code}
```{code-block} rust
struct Position {
    position: [f32; 2],
}
```

```{code-block} python
import ctypes

class Position(ctypes.Structure):
    _fields_ = [
        ("position", ctypes.c_float * 2),
    ]
```

```{code-block} c++
struct Position {
    float position[2];
};
```

```{code-block} c
struct Position {
    float position[2];
};
```
````

The server responds with Larry’s current state:

````{tab-set-code}
```{code-block} rust
struct State {
    current_position: [f32; 2],
    speed: [f32; 2],
}
```

```{code-block} python
class State(ctypes.Structure):
    _fields_ = [
        ("position", ctypes.c_float * 2),
        ("speed", ctypes.c_float * 2),
    ]
```

```{code-block} c++
struct State {
    float position[2];
    float speed[2];
};
```

```{code-block} c
struct State {
    float position[2];
    float speed[2];
};
```
````

First we create a node:

````{tab-set-code}
```{code-block} rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new().create::<ipc::Service>()?;
```

```{code-block} python
import iceoryx2 as iox2

node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)
```

```{code-block} c++
#include "iceoryx2.hpp"

using namespace iox2;

auto node = NodeBuilder().create<ServiceType::Ipc>().expect("");
```

```{code-block} c
#include "iox2/iceoryx2.h"

iox2_node_builder_h node_builder = iox2_node_builder_new(NULL);
iox2_node_h node = NULL;
if (iox2_node_builder_create(node_builder, NULL, iox2_service_type_e_IPC, &node) != IOX2_OK) {
    printf("Could not create node!\n");
    exit(-1);
}

// do not forget to release the resources later
iox2_node_drop(node);
```
````

Then the service, specifying request and response types:

````{tab-set-code}
```{code-block} rust
let service = node
    .service_builder(&"autopilot")
    .request_response::<Position, State>()
    .open_or_create()?;
```

```{code-block} python
service = (
    node.service_builder(iox2.ServiceName.new("autopilot"))
    .request_response(Position, State)
    .open_or_create()
)
```

```{code-block} c++
auto service = node.service_builder(ServiceName::create("autopilot").expect(""))
                   .request_response<Position, State>()
                   .open_or_create()
                   .expect("");
```

```{code-block} c
// Create service name
const char* service_name_value = "autopilot";
iox2_service_name_h service_name = NULL;
if (iox2_service_name_new(NULL, service_name_value, strlen(service_name_value), &service_name) != IOX2_OK) {
    printf("Unable to create service name!\n");
    exit(-1);
}

// Create service builder
iox2_service_name_ptr service_name_ptr = iox2_cast_service_name_ptr(service_name);
iox2_service_builder_h service_builder = iox2_node_service_builder(&node_handle, NULL, service_name_ptr);
iox2_service_builder_request_response_h service_builder_request_response =
    iox2_service_builder_request_response(service_builder);

// Set request and response type details
const char* request_type_name = "Position";
const char* response_type_name = "State";

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

// Create service
iox2_port_factory_request_response_h service = NULL;
if (iox2_service_builder_request_response_open_or_create(service_builder_request_response, NULL, &service)
    != IOX2_OK) {
    printf("Unable to create service!\n");
    exit(-1);
}

// do not forget to release the resources later
iox2_port_factory_request_response_drop(service);
iox2_service_name_drop(service_name);
```
````

And create the client port:

````{tab-set-code}
```{code-block} rust
let client = service.client_builder().create()?;
```

```{code-block} python
client = service.client_builder().create()
```

```{code-block} c++
auto client = service.client_builder().create().expect("");
```

```{code-block} c
iox2_port_factory_client_builder_h client_builder =
    iox2_port_factory_request_response_client_builder(&service, NULL);
iox2_client_h client = NULL;
if (iox2_port_factory_client_builder_create(client_builder, NULL, &client) != IOX2_OK) {
    printf("Unable to create client!\n");
    exit(-1);
}

// do not forget to release the resources later
iox2_client_drop(client);
```
````

Now we can send a request. The returned `pending_response` object represents the
stream of updates for this request. Dropping it automatically signals the server
to stop sending further responses.

````{tab-set-code}
```{code-block} rust
let request = client.loan_uninit()?;
let request = request.write_payload(Position {
    position: [123.456, 789.1],
});
let pending_response = request.send()?;
```

```{code-block} python
request = client.loan_uninit()
p = Position()
p.position[0] = 123.456
p.position[1] = 789.1
request = request.write_payload(p)
pending_response = request.send()
```

```{code-block} c++
auto request = client.loan_uninit().expect("");
auto initialized_request = request.write_payload(Position {{123.456, 789.1}});

pending_response = send(std::move(initialized_request)).expect("");
```

```{code-block} c
// Loan request sample
iox2_request_mut_h request = NULL;
if (iox2_client_loan_slice_uninit(&client, NULL, &request, 1) != IOX2_OK) {
    printf("Failed to loan request\n");
    exit(-1);
}

// Write payload
struct Position* payload;
iox2_request_mut_payload_mut(&request, (void**) &payload, NULL);
payload->position[0] = 123.456;
payload->position[1] = 789.1;

// Send request
iox2_pending_response_h pending_response = NULL;
if (iox2_request_mut_send(request, NULL, &pending_response) != IOX2_OK) {
    printf("Failed to send request\n");
    exit(-1);
}
```
````

In the event loop we poll every 100 ms and handle three cases:

1. User pressed “stop” → drop `pending_response` to cancel.
2. Larry disconnected before reaching the goal → show “obstacle detected.”
3. Normal update → display current position.

````{tab-set-code}
```{code-block} rust
while node.wait(Duration::from_millis(100)).is_ok() {
    while let Some(position) = pending_response.receive()? {
        show_larry_position_in_app(position);

        if !pending_response.is_connected() {
            if is_at_destination(position) {
                show_larry_arrived_popup_in_app();
            } else {
                show_larry_encountered_obstacle_in_app();
            }
        }
    }

    if user_has_pressed_stop_button() {
        drop(pending_response);
    }
}
```

```{code-block} python
try:
    while True:
        node.wait(iox2.Duration.from_millis(100))

        while True:
            position = pending_response.receive()
            if pending_response.is_connected is False:
                if is_at_destination(position):
                    show_larry_arrived_popup_in_app()
                else:
                    show_larry_encountered_obstacle_in_app()

            if position is not None:
                show_larry_position_in_app(position)
            else:
                break

except iox2.NodeWaitFailure:
    print("exit")
```

```{code-block} c++
while (node.wait(iox::units::Duration::fromMilliseconds(100)).has_value()) {
    while (true) {
        auto position = pending_response.receive().expect("");
        if (position.has_value()) {
            show_larry_position_in_app(position.value())
        } else {
            break;
        }

        if (!pending_response.is_connected()) {
            if (is_at_destination(position)) {
                show_larry_arrived_popup_in_app();
            } else {
                show_larry_encountered_obstacle_in_app();
            }
        }
    }
}
```

```{code-block} c
while (iox2_node_wait(&node_handle, 0, 10000000) == IOX2_OK) {
    iox2_response_h position = NULL;
    while (true) {
        position = NULL;
        if (iox2_pending_response_receive(&pending_response, NULL, &position) != IOX2_OK) {
            printf("Failed to receive response\n");
            exit(-1);
        }

        if ( !iox2_pending_response_is_connected(&pending_response) ) {
            if (is_at_destination(position)) {
                show_larry_arrived_popup_in_app();
            } else {
                show_larry_encountered_obstacle_in_app();
            }
        }

        if (response == NULL) {
            break;
        }

        const struct State* position_data = NULL;
        iox2_response_payload(&response, (const void**) &position_data, NULL);
        show_larry_arrived_popup_in_app(position_data);
        iox2_response_drop(position);
    }
}
```
````

## Server

On the server side, the setup looks similar:

````{tab-set-code}
```{code-block} rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new().create::<ipc::Service>()?;
let service = node
    .service_builder(&"autopilot")
    .request_response::<Position, State>()
    .open_or_create()?;

let server = service.server_builder().create()?;
```

```{code-block} python
node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)

service = (
    node.service_builder(iox2.ServiceName.new("autopilot"))
    .request_response(Position, State)
    .open_or_create()
)

server = service.server_builder().create()
```

```{code-block} c++
auto node = NodeBuilder().create<ServiceType::Ipc>().expect("");

auto service = node.service_builder(ServiceName::create("autopilot").expect(""))
                   .request_response<Position, State>()
                   .open_or_create()
                   .expect("");

auto server = service.server_builder().create().expect("");
```

```{code-block} c
// Create new node
iox2_node_builder_h node_builder_handle = iox2_node_builder_new(NULL);
iox2_node_h node_handle = NULL;
if (iox2_node_builder_create(node_builder_handle, NULL, iox2_service_type_e_IPC, &node_handle) != IOX2_OK) {
    printf("Could not create node!\n");
    exit(-1);
}

// Create service name
const char* service_name_value = "autopilot";
iox2_service_name_h service_name = NULL;
if (iox2_service_name_new(NULL, service_name_value, strlen(service_name_value), &service_name) != IOX2_OK) {
    printf("Unable to create service name!\n");
    exit(-1);
}

// Create service builder
iox2_service_name_ptr service_name_ptr = iox2_cast_service_name_ptr(service_name);
iox2_service_builder_h service_builder = iox2_node_service_builder(&node_handle, NULL, service_name_ptr);
iox2_service_builder_request_response_h service_builder_request_response =
    iox2_service_builder_request_response(service_builder);

// Set request and response type details
const char* request_type_name = "Position";
const char* response_type_name = "State";

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

// Create service
iox2_port_factory_request_response_h service = NULL;
if (iox2_service_builder_request_response_open_or_create(service_builder_request_response, NULL, &service)
    != IOX2_OK) {
    printf("Unable to create service!\n");
    exit(-1);
}

// Create server
iox2_port_factory_server_builder_h server_builder =
    iox2_port_factory_request_response_server_builder(&service, NULL);
iox2_server_h server = NULL;
if (iox2_port_factory_server_builder_create(server_builder, NULL, &server) != IOX2_OK) {
    printf("Unable to create server!\n");
    exit(-1);
}

// do not forget to release the resources later
iox2_server_drop(server);
iox2_port_factory_request_response_drop(service);
iox2_service_name_drop(service_name);
iox2_node_drop(node_handle);
```
````

The server checks every 100 ms for new requests. (You could combine this with
event messaging to block until a request arrives, but for the purpose of this
tutorial, we keep it simple.)
When a request is received, Larry starts driving and periodically sends status
updates.

If an obstacle is detected, or the client disconnects, the active request is
dropped and Larry stops.

````{tab-set-code}
```{code-block} rust
let mut active_request = None;
while node.wait(Duration::from_millis(100)).is_ok() {
    if active_request.is_none() {
        if let Some(new_request) = server.receive()? {
            drive_to_position(&new_request);
            active_request = Some(new_request);
        }
    }

    if let Some(current_request) = &active_request {
        if !current_request.is_connected() {
            active_request = None;
            stop_driving();
        } else if arrived_at_destination() {
            active_request = None;
        } else {
            let response = current_request.loan_uninit()?;
            let response = response.write_payload(get_current_state());
            response.send()?;
        }
    }
}
```

```{code-block} python
try:
    while True:
        node.wait(iox2.Duration.from_millis(100))
        if active_request is None:
            active_request = server.receive()

            if active_request is not None:
                drive_to_position(active_request)

        if active_request is not None:
            if active_request.is_connected is False:
                stop_driving()
                active_request = None
            else if arrived_at_destination():
                active_request = None
            else:
                response = active_request.loan_uninit()
                response = response.write_payload(get_current_state())
                response.send()

except iox2.NodeWaitFailure:
    print("exit")
```

```{code-block} c++
auto active_request = server.receive().expect("");
while (node.wait(iox::units::Duration::fromMilliseconds(100)).has_value()) {
    if ( !active_request.has_value() ) {
        active_request = server.receive().expect("");
        if ( active_request.has_value() ) {
            drive_to_position(active_request);
        }
    }

    if ( active_request.has_value() ) {
        if ( !active_request->is_connected() ) {
            stop_driving();
            active_request = iox::nullopt;
        } else if ( arrived_at_destination() ) {
            active_request = iox::nullopt;
        } else {
            auto response = active_request->loan_uninit().expect("");
            auto initialized_response = response.write_payload(get_current_state());
            send(std::move(initialized_response)).expect("");
        }
    }
}
```

```{code-block} c
iox2_active_request_h active_request = NULL;
while (iox2_node_wait(&node_handle, 0, 10000000) == IOX2_OK) {
    if ( active_request == NULL ) {
        if (iox2_server_receive(&server, NULL, &active_request) != IOX2_OK) {
            printf("Failed to receive request\n");
            exit(-1);
        }

        if ( active_request != NULL ) {
            drive_to_position(active_request);
        }
    }

    if ( active_request != NULL ) {
        if ( !iox2_active_request_is_connected(&active_request) ) {
            stop_driving();
            iox2_active_request_drop(active_request);
        } else if ( arrived_at_destination() ) {
            iox2_active_request_drop(active_request);
        } else {
            iox2_response_mut_h response = NULL;
            if (iox2_active_request_loan_slice_uninit(&active_request, NULL, &response, 1) != IOX2_OK) {
                printf("Failed to loan response sample\n");
                exit(-1);
            }

            struct TransmissionData* state = NULL;
            iox2_response_mut_payload_mut(&response, (void**) &state, NULL);
            get_current_state(state);
            if (iox2_response_mut_send(response) != IOX2_OK) {
                printf("Failed to send additional response\n");
                exit(-1);
            }
        }
    }
}
```
````

This way, the client can receive a continuous stream of updates, either side
can gracefully stop when necessary, and Larry doesn’t drive blindly into
canyons (in theory).

## Related Examples

<!-- markdownlint-disable-next-line MD033 -->
<br/>

````{grid} 1 1 2 2
:gutter: 3

```{grid-item}
**{octicon}`code` Rust**

{octicon}`mark-github` [Minimal Client](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/request_response/client.rs)  
{octicon}`mark-github` [Minimal Server](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/request_response/server.rs)  
```

```{grid-item}
**{octicon}`code` Python**

{octicon}`mark-github` [Minimal Client](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/request_response/client.py)  
{octicon}`mark-github` [Minimal Server](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/request_response/server.py)  
```

```{grid-item}
**{octicon}`code` C++**

{octicon}`mark-github` [Minimal Client](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/request_response/src/client.cpp)  
{octicon}`mark-github` [Minimal Server](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/request_response/src/server.cpp)  
```

```{grid-item}
**{octicon}`code` C**

{octicon}`mark-github` [Minimal Client](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/request_response/src/client.c)  
{octicon}`mark-github` [Minimal Server](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/request_response/src/server.c)  
```

````
