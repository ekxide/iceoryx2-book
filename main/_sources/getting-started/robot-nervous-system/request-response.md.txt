# Request-Response

With emergency brakes and some basic fault handling in place, it’s time for
something a bit more sophisticated.

Imagine the user wants to send our robot Larry to a specific position:
“Go to point B.”
As Larry starts driving, the user expects status updates - position, speed,
progress. At some point the user may even cancel the trip, or Larry himself may
have to abort if he encounters, say, a canyon.

```{inline-svg} /images/robot-nervous-system-request-response-sequence.svg
:name: fig-wiring-a-robot-brain-request-response-sequence
:alt: Request-Response Sequence Diagram between App and Larry Autopilot

Request-Response interactions between App and Larry Autopilot
```

This is where the request-response (stream) pattern in `iceoryx2` comes into
play. It behaves like regular non-blocking request-response, but with one extra
trick: the server can send a stream of responses instead of just one. Either
side can end the stream at any time if it’s no longer interested.

## Client

The client is our user-facing app. The request can be a simple 2D position:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/request-response/rust/src/autopilot.rs
:language: rust
:start-after: snippet:start request-type
:end-before: snippet:end request-type
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/python/autopilot.py
:language: python
:start-after: snippet:start request-type
:end-before: snippet:end request-type
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/cxx/src/autopilot.hpp
:language: c++
:start-after: snippet:start request-type
:end-before: snippet:end request-type
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/c/src/autopilot.h
:language: c
:start-after: snippet:start request-type
:end-before: snippet:end request-type
:dedent:
```
````

The server responds with Larry’s current state:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/request-response/rust/src/autopilot.rs
:language: rust
:start-after: snippet:start response-type
:end-before: snippet:end response-type
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/python/autopilot.py
:language: python
:start-after: snippet:start response-type
:end-before: snippet:end response-type
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/cxx/src/autopilot.hpp
:language: c++
:start-after: snippet:start response-type
:end-before: snippet:end response-type
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/c/src/autopilot.h
:language: c
:start-after: snippet:start response-type
:end-before: snippet:end response-type
:dedent:
```
````

First we create a node:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/request-response/rust/src/bin/client.rs
:language: rust
:start-after: snippet:start node
:end-before: snippet:end node
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/python/client.py
:language: python
:start-after: snippet:start node
:end-before: snippet:end node
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/cxx/src/client.cpp
:language: c++
:start-after: snippet:start node
:end-before: snippet:end node
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/c/src/client.c
:language: c
:start-after: snippet:start node
:end-before: snippet:end node
:dedent:
```
````

Then the service, specifying request and response types:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/request-response/rust/src/bin/client.rs
:language: rust
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/python/client.py
:language: python
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/cxx/src/client.cpp
:language: c++
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/c/src/client.c
:language: c
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```
````

And create the client port:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/request-response/rust/src/bin/client.rs
:language: rust
:start-after: snippet:start client
:end-before: snippet:end client
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/python/client.py
:language: python
:start-after: snippet:start client
:end-before: snippet:end client
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/cxx/src/client.cpp
:language: c++
:start-after: snippet:start client
:end-before: snippet:end client
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/c/src/client.c
:language: c
:start-after: snippet:start client
:end-before: snippet:end client
:dedent:
```
````

Now we can send a request. The returned `pending_response` object represents the
stream of updates for this request. Dropping it automatically signals the server
to stop sending further responses.

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/request-response/rust/src/bin/client.rs
:language: rust
:start-after: snippet:start send-request
:end-before: snippet:end send-request
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/python/client.py
:language: python
:start-after: snippet:start send-request
:end-before: snippet:end send-request
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/cxx/src/client.cpp
:language: c++
:start-after: snippet:start send-request
:end-before: snippet:end send-request
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/c/src/client.c
:language: c
:start-after: snippet:start send-request
:end-before: snippet:end send-request
:dedent:
```
````

In the event loop we poll every 100 ms and handle three cases:

1. User pressed “stop” → drop `pending_response` to cancel.
2. Larry disconnected before reaching the goal → show “obstacle detected.”
3. Normal update → display current position.

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/request-response/rust/src/bin/client.rs
:language: rust
:start-after: snippet:start event-loop
:end-before: snippet:end event-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/python/client.py
:language: python
:start-after: snippet:start event-loop
:end-before: snippet:end event-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/cxx/src/client.cpp
:language: c++
:start-after: snippet:start event-loop
:end-before: snippet:end event-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/c/src/client.c
:language: c
:start-after: snippet:start event-loop
:end-before: snippet:end event-loop
:dedent:
```
````

## Server

On the server side, the setup looks similar:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/request-response/rust/src/bin/server.rs
:language: rust
:start-after: snippet:start setup
:end-before: snippet:end setup
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/python/server.py
:language: python
:start-after: snippet:start setup
:end-before: snippet:end setup
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/cxx/src/server.cpp
:language: c++
:start-after: snippet:start setup
:end-before: snippet:end setup
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/c/src/server.c
:language: c
:start-after: snippet:start setup
:end-before: snippet:end setup
:dedent:
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
```{literalinclude} ../../../snippets/robot-nervous-system/request-response/rust/src/bin/server.rs
:language: rust
:start-after: snippet:start serve-loop
:end-before: snippet:end serve-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/python/server.py
:language: python
:start-after: snippet:start serve-loop
:end-before: snippet:end serve-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/cxx/src/server.cpp
:language: c++
:start-after: snippet:start serve-loop
:end-before: snippet:end serve-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/request-response/c/src/server.c
:language: c
:start-after: snippet:start serve-loop
:end-before: snippet:end serve-loop
:dedent:
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
