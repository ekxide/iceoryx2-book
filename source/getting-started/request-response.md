# Request–Response

With emergency brakes and some basic fault handling in place, it’s time for
something a bit more sophisticated.

Imagine the user wants to send our robot Larry to a specific position:
“Go to point B.”
As Larry starts driving, the user expects status updates—position, speed,
progress. At some point the user may even cancel the trip, or Larry himself may
have to abort if he encounters, say, a canyon.

This is where the request–response (stream) pattern in iceoryx2 comes into
play. It behaves like regular non-blocking request–response, but with one extra
trick: the server can send a stream of responses instead of just one. Either
side can end the stream at any time if it’s no longer interested.

```rust
Client (App)                           Server (Larry Autopilot)
     |                                             |
     |--- Request: "Go to point B" --------------->|
     |                                             |
     |<-- Response: "Starting drive..." -----------|
     |<-- Response: "At (10, 5), speed (1, 0)" ----|
     |<-- Response: "At (20, 5), speed (1, 0)" ----|
     |<-- Response: "At (30, 5), speed (0, 0)" ----|
     |<-- Response: "Arrived at destination!" -----|
     |                                             |
     |--- Cancel signal (drop pending response) -->|
     |                                             |
```

## Client

The client is our user-facing app. A request could be a simple 2D position:

```rust
struct Position {
    position: [f32; 2],
}
```

The server responds with Larry’s current state:

```rust
struct State {
    current_position: [f32; 2],
    speed: [f32; 2],
}
```

First we create a node:

```rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new().create::<ipc::Service>()?;
```

Then the service, specifying request and response types:

```rust
let service = node
    .service_builder(&"Autopilot")
    .request_response::<Position, State>()
    .open_or_create()?;
```

Create the client port:

```rust
let client = service.client_builder().create()?;
```

Now we can send a request. The returned `pending_response` object represents the
stream of updates for this request. Dropping it automatically signals the server
to stop sending further responses.

```rust
let request = client.loan_uninit()?;
let request = request.write_payload(Position {
    position: [123.456, 789.1],
});
let pending_response = request.send()?;
```

In the event loop we poll every 100 ms and handle three cases:

1. User pressed “stop” → drop `pending_response` to cancel.
2. Larry disconnected before reaching the goal → show “obstacle detected.”
3. Normal update → display current position.

```rust
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

## Server

On the server side, setup looks similar:

```rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new().create::<ipc::Service>()?;
let service = node
    .service_builder(&"Autopilot")
    .request_response::<Position, State>()
    .open_or_create()?;

let server = service.server_builder().create()?;
```

The server checks every 100 ms for new requests. (You could combine this with
event messaging to block until a request arrives, but here we keep it simple.)
When a request is received, Larry starts driving and periodically sends status
updates.

If an obstacle is detected, or the client disconnects, the active request is
dropped and Larry stops.

```rust
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

This way, the client gets a continuous stream of updates, either side can
gracefully stop when necessary, and Larry doesn’t drive blindly into canyons
(in theory).

## Source Code

````{tab-set}
```{tab-item} C
* [GitHub C Client-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/request_response/src/client.c)
* [GitHub C Server-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/request_response/src/server.c)
```

```{tab-item} C++
* [GitHub C++ Client-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/request_response/src/client.cpp)
* [GitHub C++ Server-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/request_response/src/server.cpp)
```

```{tab-item} PYTHON
* [GitHub Python Client-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/request_response/client.py)
* [GitHub Python Server-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/request_response/server.py)
```

```{tab-item} RUST
* [GitHub Rust Client-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/request_response/client.rs)
* [GitHub Rust Server-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/request_response/server.rs)
```
````
