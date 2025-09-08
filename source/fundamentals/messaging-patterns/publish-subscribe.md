# Publish-Subscribe

```{figure} /images/publish-subscribe-pattern.svg
:alt: publish-subscribe messaging pattern
:align: center
:name: fig-publish-subscribe-communication

Publish-Subscribe Communication Model
```

A popular unidirectional, multi-producer, multi-consumer messaging pattern
that decouples the producer (publisher) from the consumer (subscriber).
This enables each participant to have their own lifecycle and operate
completely independently. Publishers send messages without knowing who will
receive them, while subscribers express interest in receiving payloads from a
service and are automatically connected to matching publishers.

The publish-subscribe messaging pattern excels at communicating large payloads
to multiple participants. The zero-copy capability enables essentially
constant latency regardless of payload size.

The pattern is not optimal for cases where data must be kept available to
participants indefinitely ([blackboard](
/fundamentals/messaging-patterns/blackboard)) or when two-way communication is
required ([request-response](
/fundamentals/messaging-patterns/request-response)).

## Mechanism

```{figure} /images/publish-subscribe-mechanism.svg
:alt: publish-subscribe messaging protocol
:align: center
:name: fig-publish-subscribe-mechanism

Publish-Subscribe over Shared Memory
```

The publish-subscribe messaging pattern is implemented using data structures in
shared memory. The following structures are involved:

1. **Payload Segment**
    * A region in shared memory used to communicate payload data between
      participants
    * Publishers have a payload segment to share samples with subscribers
    * The organization of memory in the payload segment depends on the
      allocator used in the `iceoryx2` deployment
        * This flexibility allows different deployments (e.g. desktop
          applications vs. safety-critical applications) to utilize a
          different strategy appropriate to their use-case
1. **Offset Channel**
    * A channel for communicating the offsets of payloads within the payload
      segment
    * Each publisher-subscriber pair has their own offset channel

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Understand Events
:link: /fundamentals/messaging-patterns/event
:link-type: doc

See how the `Event` messaging pattern can be used to coordinate participants.
```

```{grid-item-card} Example: Publish-Subscribe Service (Rust)
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/rust/publish_subscribe
:link-type: url

See how to set up a basic `PublishSubscribe` service.
```

```{grid-item-card} Example: Dynamic Publish-Subscribe Service (Rust)
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/rust/publish_subscribe_dynamic_data
:link-type: url

See how to set up a `PublishSubscribe` service with dynamic memory allocations.
```

````
