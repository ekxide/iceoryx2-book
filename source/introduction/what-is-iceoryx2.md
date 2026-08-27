# What is iceoryx2?

## A Zero-Copy Data Plane

`iceoryx2` is the second generation of the Eclipse iceoryx open-source project, built as a **zero-copy data plane** for data-intensive and mission-critical systems. It provides efficient and deterministic zero-copy communication between software components with predictable, low latency and minimal communication overhead.

As a lean communication foundation, `iceoryx2` can be integrated into existing architectures without imposing a specific data model, execution model, or ecosystem. It focuses on moving data efficiently between software components while leaving the surrounding system architecture under your control.

## True Zero-Copy Communication

For inter-process communication (IPC) on the same system, `iceoryx2` uses **true zero-copy communication based on shared memory**.

Instead of copying a payload from one process to another, the producer writes the data directly into shared memory that can be accessed by the consumers. When the data is published, only references to this memory are exchanged. This keeps data-delivery latency independent of payload size and allows high-volume local data exchange to scale efficiently.

```{inline-svg} /images/zero-copy-data-flow.svg
:alt: True Zero-Copy Communication

True Zero-Copy Communication
```

`iceoryx2` takes care of the communication infrastructure around this mechanism, including discovery, message delivery, memory ownership, and synchronization.

## Messaging Patterns

In data-intensive systems such as physical AI, communication and execution need to be orchestrated together. `iceoryx2` separates **data flow from control flow** and supports a variety of [messaging patterns](/fundamentals/messaging-patterns/index), including publish-subscribe, request-response, events, and a shared-memory blackboard.

Publish-subscribe and request-response use configurable queues with quality-of-service settings for queue sizes and backpressure strategies. Publish-subscribe additionally supports a configurable publisher message history for late-joining subscribers.

This separation of data flow and control flow, together with the provided communication mechanisms, allows fine-grained control over when execution happens and which data is made available.

## Language Bindings and Operating Systems

`iceoryx2` is implemented in Rust and provides additional language bindings for C, C++, Python, and C#. Applications written in different languages can communicate with each other through the same `iceoryx2` services.

With memory-compatible data types, this also enables **true zero-copy communication across language boundaries**. Instead of serializing data between, for example, a Rust producer and a C++ consumer, both applications can access the same representation directly in shared memory.

`iceoryx2` supports a broad range of operating systems, including Linux, Windows, macOS, QNX, and FreeBSD. Additional platforms are supported experimentally or with a more limited level of continuous testing.

This allows the same communication concepts and APIs to be used across development machines and embedded production targets.

## Expanding the Local Data Plane

The zero-copy data plane focuses on efficient local communication, but many systems also need to communicate beyond a single machine or integrate with existing data models and IDLs.

For this, `iceoryx2` provides extension points for integration with external data models and IDLs, as well as communication beyond a single machine through tunnels and gateways with pluggable network protocols.

This allows `iceoryx2` to remain a lean local data plane while integrating into larger distributed and heterogeneous system architectures.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Fundamentals: Shared Memory
:link: /fundamentals/shared-memory
:link-type: doc
:shadow: none

Learn more about how `iceoryx2` leverages shared memory.
```

```{grid-item-card} Fundamentals: Messaging Patterns
:link: /fundamentals/messaging-patterns/index
:link-type: doc
:shadow: none

Learn more about the supported messaging patterns.
```

```{grid-item-card} Fundamentals: Cross-Language Support
:link: /fundamentals/cross-language-support
:link-type: doc
:shadow: none

Learn more about cross-language communication.
```

````
