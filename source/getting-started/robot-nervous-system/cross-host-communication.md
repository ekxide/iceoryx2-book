# Cross-host communication

```{admonition} Learning Objectives
This tutorial extends an `iceoryx2` system across host boundaries using the
tunnel:

1. Enable cross-host communication with the `iox2 tunnel` CLI, without
   touching application code
2. Embed the tunnel in your own application for full control over its execution
3. Drive the tunnel in polling and reactive modes
4. Switch the communication mechanism by instantiating a different backend
```

So far Larry has been a self-contained system: his sensors, his control logic,
and his actuators all reside on one host and talk over `iceoryx2`'s
shared memory. But shared memory does not extend beyond the boundary of the
host.

Imagine now that the user wants to remotely track **where** Larry is and how
much **battery** he has left. Some kind of dashboard hosted on another
device on the same network.

The required data is already flowing through `iceoryx2`'s shared memory. A
[gateway or a tunnel](/fundamentals/gateways-and-tunnels) can hook into that
flow and propagate it to other hosts. To get the data to the dashboard,
we will use a network tunnel, which propagates the raw shared memory payloads
over the network.

```{tip}
A gateway or tunnel can be implemented for any communication mechanism, not
just network communication. All that is required is an implementation of
the `Backend` traits available in [`iceoryx2-services-tunnel-backend`](
https://docs.rs/iceoryx2-services-tunnel-backend/0.9.1/iceoryx2_services_tunnel_backend/).

For example, the tunnel has been used to extend communication between
co-processors on a single board that have their own memory using cross-chip
communication APIs.
```

## Application setup

As in previous articles in this series, the data flowing through Larry are
defined as `ZeroCopySend` structs:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/cross-host-communication/rust/src/telemetry_data.rs
:language: rust
:start-after: snippet:start
:end-before: snippet:end
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/cross-host-communication/python/telemetry_data.py
:language: python
:start-after: snippet:start
:end-before: snippet:end
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/cross-host-communication/cxx/src/telemetry_data.hpp
:language: c++
:start-after: snippet:start
:end-before: snippet:end
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/cross-host-communication/c/src/telemetry_data.h
:language: c
:start-after: snippet:start
:end-before: snippet:end
:dedent:
```
````

For demonstration purposes, we use the [event-driven communication](
event-driven-communication.md) execution pattern, which is the same approach
used by many popular network communication libraries. However, with `iceoryx2`,
you are free to choose the best approach that suits your application — the
tunnel can be configured to work with any approach. See [execution control
patterns](/tutorials/execution-control-patterns) for an overview.

```{note}
The tunnel currently only supports the publish-subscribe and event messaging
patterns. Support for request-response and blackboard is planned.
```

We set Larry up to send a notification on a corresponding event service along
with every sample that is published:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/cross-host-communication/rust/src/bin/larry_telemetry.rs
:language: rust
:start-after: snippet:start
:end-before: snippet:end
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/cross-host-communication/python/larry_telemetry.py
:language: python
:start-after: snippet:start
:end-before: snippet:end
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/cross-host-communication/cxx/src/larry_telemetry.cpp
:language: c++
:start-after: snippet:start
:end-before: snippet:end
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/cross-host-communication/c/src/larry_telemetry.c
:language: c
:start-after: snippet:start
:end-before: snippet:end
:dedent:
```
````

In the dashboard application running on another host, we set up listeners to
wake the thread when new data arrives, and subscribers to receive the data.
A [`WaitSet`](/fundamentals/execution-control.md#waitset) is used to react
to events from both listeners in a single thread.

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/cross-host-communication/rust/src/bin/dashboard.rs
:language: rust
:start-after: snippet:start
:end-before: snippet:end
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/cross-host-communication/python/dashboard.py
:language: python
:start-after: snippet:start
:end-before: snippet:end
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/cross-host-communication/cxx/src/dashboard.cpp
:language: c++
:start-after: snippet:start
:end-before: snippet:end
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/cross-host-communication/c/src/dashboard.c
:language: c
:start-after: snippet:start
:end-before: snippet:end
:dedent:
```
````

Neither application is aware that the data will be propagated over the network.
They are both set up as regular `iceoryx2` applications.

## Tunneling over the network

With the applications set up, we now need to spin up the tunnel to extend
the communication over the network. The tunnel can either be run in a separate
process or embedded into the application. First, let's use the `iceoryx2-cli`
to run the tunnel in a separate process.

### Installing the CLI

First install the [`iceoryx2-cli`](https://crates.io/crates/iceoryx2-cli)
which is the entry point for all CLI commands:

```sh
cargo install iceoryx2-cli
```

Next, the CLI for a specific tunnel backend must be installed. For this
article, we use the Zenoh backend:

```sh
cargo install iceoryx2-integrations-zenoh-tunnel-cli
```

The command `iox2 tunnel --list` can be used to verify that the installation
was successful. This command lists all installed tunnel backends discovered
by the CLI.

The tunnel should be installed on both hosts.

### Running a tunnel process

The CLI can now be used to start a tunnel process on both hosts:

```console
iox2 tunnel zenoh
```

Each tunnel discovers `iceoryx2` services, both on its own host and over
the communication mechanism used by the chosen backend implementation.
Larry's tunnel sees `larry/battery` and `larry/position` and forwards all
samples published to them over the wire. The dashboard tunnel receives these
samples from the wire, and injects them into the `iceoryx2` shared memory to be
received by subscribers to matching services on the dashboard host. The two
applications communicate over services, as if they were running on the same
host.

By default the tunnel polls for new services and pending samples every 100 ms,
however since the event-based communication pattern is used, it is possible
to configure the tunnel to execute reactively.

The tunnel running on Larry only has work to do when new samples are published
on the services that need to be tunneled:

```console
iox2 tunnel zenoh --listener "larry/battery" --listener "larry/position"
```

The tunnel running on the dashboard host only has work to do when data arrives
over the backend:

```console
iox2 tunnel zenoh --reactive-backend
```

Nothing here is application code. Both applications were written, compiled,
and deployed with no knowledge that a tunnel exists. Communicating across the
hosts is purely a deployment decision.

### Embedding the tunnel

```{note}
The embeddable tunnel is currently only available as a Rust library. C++, C,
and Python applications must use the tunnel from a separate process with the
`iox2 tunnel` CLI shown above; the remainder of this section applies to Rust.
```

Running the tunnel as its own process is convenient, but it's not the only
option. The tunnel is a library, and it spawns no threads of its own so you
have full control over when and where to run it — it can be executed within a
dedicated time slot of a time-triggered architecture, or on a separate
thread that you control, pinned to a specific core so as not to interfere
with application execution.

First add the generic tunnel and your chosen backend as dependencies:

```toml
iceoryx2-services-tunnel = { version = "X.Y.Z" }
iceoryx2-integrations-zenoh-tunnel-backend = { version = "X.Y.Z" }
```

The tunnel can then be instantiated in your application via the provided
builder. The chosen backend implementation is passed as a generic on creation,
here we again use the `ZenohBackend`, but any other implementation can be
dropped in, even your own. Specifying `polled()` in the build returns a tunnel
that can be manually driven:

```{literalinclude} ../../../snippets/robot-nervous-system/cross-host-communication/rust/src/bin/embedded_polled.rs
:language: rust
:start-after: snippet:start
:end-before: snippet:end
:dedent:
```

Execution is driven via two API calls. The `discover()` call reconciles which
services exist locally and over the wire. The `propagate()` call moves the
data in both directions: from the wire into `iceoryx2` shared memory and vice
versa. Calling the two on the polling tunnel in a timed loop
replicates the default behavior of the CLI.

Now let's take a look at how to replicate the reactive configurations provided
by the tunnel. Recapping the details, both tunnels must be set up to react
to different things:

1. Larry's tunnel reacts to the services notified when a sample is published
1. The dashboard's tunnel reacts to data arriving over the network

Since Larry's tunnel does not react to network communication, it can be
created in polling mode. Listeners are created for the services notified with
each publish, and a [`WaitSet`](/fundamentals/execution-control.md#waitset) is
set up to react to notifications from both of them in a single thread:

```{literalinclude} ../../../snippets/robot-nervous-system/cross-host-communication/rust/src/bin/embedded_reactive_sender.rs
:language: rust
:start-after: snippet:start
:end-before: snippet:end
:dedent:
```

The dashboard tunnel only cares about data arriving over the network, so it is
created in reactive mode, which provides a listener along with the created
tunnel. This listener is notified when new data is available from the backend
specified on creation.
No [`WaitSet`](/fundamentals/execution-control.md#waitset) is needed here,
since there is only one listener to wait on:

```{literalinclude} ../../../snippets/robot-nervous-system/cross-host-communication/rust/src/bin/embedded_reactive_receiver.rs
:language: rust
:start-after: snippet:start
:end-before: snippet:end
:dedent:
```

Both approaches run the same `discover()` and `propagate()` calls; they differ
only in what decides when those calls happen. Once again, the tunnel has no
background threads, so you are free to choose where and when these operations
run, allowing you to plan for and minimize unpredictability.

## Switching the mechanism

Let's say that later in development you decide a different mechanism should
carry the cross-host communication. Perhaps you want to reach the dashboard
over the internet by integrating with existing MQTT infrastructure, or you
need to bridge co-processors on a single board that each keep their own memory
and can only communicate over a cross-chip API.

All that is required, is to switch out the chosen backend when creating the
tunnel. In our examples above, we have been using Zenoh, so the tunnels were
created with:

```rust
Tunnel::<ipc::Service, ZenohBackend<ipc::Service>>
```

The first parameter is the local `iceoryx2` service type; the second is the
backend. The tunnel itself knows nothing about Zenoh. It is written against the
backend traits, and `discover()` and `propagate()` are the same calls
regardless of what carries the bytes.

Switching over to a different mechanism is therefore only a change of the type
specified on tunnel creation:

```rust
Tunnel::<ipc::Service, SomeOtherBackend<ipc::Service>>
```

The publishers, the subscribers, the embed loops, and the choice of polled or
reactive mode are all untouched.

Over time, additional backend implementations
will be provided upstream alongside `iceoryx2`, however if there isn't an
existing implementation for the mechanism you would like to use, it is possible
to implement your own using the traits provided in [`iceoryx2-services-tunnel-backend`](
https://docs.rs/iceoryx2-services-tunnel-backend/0.9.1/iceoryx2_services_tunnel_backend/).

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Gateways and Tunnels
:link: /fundamentals/gateways-and-tunnels
:link-type: doc
:shadow: none

The feature that this tutorial puts to work.
```

```{grid-item-card} Event-Driven Communication
:link: /getting-started/robot-nervous-system/event-driven-communication
:link-type: doc
:shadow: none

The control-flow pattern the dashboard and tunnel employ.
```

```{grid-item-card} Execution Control Patterns
:link: /tutorials/execution-control-patterns
:link-type: doc
:shadow: none

Overview of execution patterns possible with `iceoryx2`.
```

````
