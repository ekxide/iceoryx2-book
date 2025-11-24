# Blackboard

```{figure} /images/blackboard-pattern.svg
:alt: blackboard messaging pattern
:align: center
:name: fig-blackboard-communication

Blackboard Communication Model
```

An unidirectional messaging pattern based on a shared-memory key-value
repository which can be modified by (for now) one writer and read by many
readers. Updates and read operations are made on a key basis, not on the entire
shared memory. All keys are unique and have the same shared-memory compatible
type; the shared-memory compatible value types can vary, but are fixed for a
specific key.

The blackboard messaging pattern is ideal for scenarios where a large number of
readers is interested in a subset of the key-value pairs, such as global
configuration settings adjusted at runtime or a regularly updated global state,
with each participant only interested in a small part of it.

The pattern is not suitable for cases where large payloads shall be
communicated to multiple participants ([publish-subscribe](
/fundamentals/messaging-patterns/publish-subscribe)) or when a bidirectional
communication is required ([request-response](
/fundamentals/messaging-patterns/request-response)).

## Mechanism

```{figure} /images/blackboard-mechanism.svg
:alt: blackboard messaging pattern
:align: center
:name: fig-blackboard-mechanism

Blackboard in Shared Memory
```

Writers and readers don't work directly on the key-value repository, but can
retrieve entry handles for a specific key, enabling efficient access. These
entry handles are used to update the corresponding value or read the latest
value without having to iterate over the entire key-value repository again.

The blackboard can be combined with the event messaging pattern so that the
receiving participant is notified when the value it is interested in is
updated. Each entry handle provides an entry id to facilitate event handling by
the receiving participants.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Understand Events
:link: /fundamentals/messaging-patterns/event
:link-type: doc

See how the `Event` messaging pattern can be used to coordinate participants.
```

```{grid-item-card} Example: Blackboard Service (Rust)
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/rust/blackboard
:link-type: url

See how to set up a basic `Blackboard` service.
```

```{grid-item-card} Example: Blackboard Service with Notification (Rust)
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/rust/blackboard_event_based_communication
:link-type: url

See how to set up a `Blackboard` service with notification.
```

````
