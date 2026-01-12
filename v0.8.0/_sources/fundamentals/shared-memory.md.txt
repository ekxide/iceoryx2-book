# Shared Memory

Shared memory is a memory region that multiple participants can access
directly, enabling the exchange of data without copying between separate
memory locations.

```{figure} /images/shared-memory.svg
:alt: shared memory region between participants
:align: center
:name: fig-shared-memory-between-participants

Memory Regions Accessible to Communicating Participants
```

It can be implemented within a single process, where participants share the
same virtual address space, or across multiple processes, where physical
memory segments are mapped into each participant's virtual address space.

Communication over shared memory provides the lowest latency and computation
overhead, which essentially remains constant regardless of payload size,
although caching behaviour may introduce some variance.

## Organization

In its basic, unstructured form, shared memory is not ergonomic to work with.
It provides only a raw block of bytes with no built-in structure, type safety,
or data organization. Developers must manually manage memory layout concerns
like alignment and padding, and implement their own data structures and
protocols for organizing and accessing data within the shared region.

`iceoryx2` takes over this responsibility so that developers don't need to,
enabling them to focus on the domain-specific challenges of their applications.

## Synchronization

Shared memory requires careful use of synchronization mechanisms
to coordinate access and ensure that data remains in a coherent state.
Implementing and employing these synchronization mechanisms correctly can be
challenging, as improper coordination between participants can lead to race
conditions, data corruption, performance degradation, or even system deadlock.

`iceoryx2` tackles the challenge of synchronizing participants by employing
battle-tested **lock-free algorithms** originating from classic `iceoryx`,
mitigating the risk of deadlock.

In the vast majority of use-cases, lock-free algorithms are sufficient.
However, for applications with extreme safety constraints,
**wait-free alternatives** can be easily substituted in. These variants are
not available open-source.

## Zero-copy

Zero-copy communication is data exchange between participants without
serialization or duplication of payload data in memory. The consumer accesses
the payload data at the exact location and form that it is produced.

While shared memory enables zero-copy communication, it requires an appropriate
API that allows producers to write data directly into shared memory regions.

```{figure} /images/zero-copy-communication.svg
:alt: zero-copy communication
:align: center
:name: fig-shared-memory-zero-copy-communication

Participants Communicating over a Shared Memory Region
```

This contrasts with alternative communication mechanisms, which can involve
serialization and multiple copies along the communication path between
participants.

```{figure} /images/network-stack-communication.svg
:alt: communication over network stack
:align: center
:name: fig-network-stack-communication-single-host

Participants Communicating over the Network Stack
```

Consider communication via the network stack, which is a common approach taken
to inter-process communication. It typically involves serialization of the
payload and copying between user buffers and network stack buffers. This ends
up requiring multiple times more memory than the actual payload size and
costing more computing power. This increases in proportion to the number of
participants.

Furthermore, while shared memory only interacts with the kernel during
creation, mapping, or deletion, communicating over mechanisms such as
the network stack often involves multiple kernel interactions during the
communication, which results in more overhead and thus communication latency.

Zero-copy communication significantly reduces both memory usage and processing
overhead, improving performance especially for large data transfers.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Understand the Communication Model
:link: /fundamentals/communication-model
:link-type: doc
:shadow: none

Get familiar with the components involved with establishing communication.
```

```{grid-item-card} Understand Publish-Subscribe
:link: /fundamentals/messaging-patterns/publish-subscribe
:link-type: doc
:shadow: none

See how the `PublishSubscribe` messaging pattern is implemented
with shared memory.
```

```{grid-item-card} Understand Request-Response
:link: /fundamentals/messaging-patterns/request-response
:link-type: doc
:shadow: none

See how the `RequestResponse` messaging pattern is implemented
with shared memory.
```
````
