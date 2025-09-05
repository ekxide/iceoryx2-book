# Request Response

So the emergency breaks are up and running and when a process dies we have
some basic safeguards in place. With this already implemented we can can more
complex challenges.

Let's assume the user would like to sends our robot Larry somewhere and defines
a position and send the request got to point B. When Larry starts working on
the request, the user wants of course a steady stream of status updates. At
some point the user even might decide that they no longer want larry to drive
to point B and abort the request. On the other hand, Larry might encounter a
large canyon towards point B, is unable to reach it and needs to abort the
operation.

Here iceoryx2 messaging pattern request response (stream) fits in perfectly. It
works like a normal non-blocking request-response communication but with the
advanced feature that the server can send a whole stream of responses. If the
client is no longer interested in the responses, it can easily signal this to
the server and the server can do the same when it can no longer deliver further
responses.

## Client

The client can be our little user-facing app where we can define to which
position larry shall drive. So the request type could be just a
2D point like:

```rust
struct Position {
    position: [f32; 2]
}
```

When larry starts driving, we might want to know its vector speed and current
position. So we define a response type of

```rust
struct State {
    current_position: [f32; 2],
    speed: [f32; 2]
}
```

We start as always and create a node first.

```rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new().create::<ipc::Service>()?;
```

In the next step we define a request-response service with `Position` as
request type and `State` as response type.

```rust
let service = node
    .service_builder(&"Autopilot")
    .request_response::<Position, State>()
    .open_or_create()?;
```

The client-port can be used to send a request to one or many servers, we create
the port like:

```rust
let client = service.client_builder().create()?;
```

In our little example we just send the request to the server to drive to
position `[123.456, 789.1]`. As soon as we send out the request we retrieve
a pending response object which can be used to receive all responses for this
specific request. When it goes out-of-scope it automatically signals the server
that the client is no longer interested in the data and that the server can
stop sending further responses.

```rust
let request = client.loan_uninit()?;
let request = request.write_payload(Position {
    position: [123.456, 789.1]
});
let pending_response = request.send()?;
```
