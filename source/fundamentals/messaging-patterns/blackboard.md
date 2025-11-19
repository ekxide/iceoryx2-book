# Blackboard

- TODO: add diagram "Blackboard Communication Model"

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

The pattern is not suitable for cases where the readers require previous values
or when a bidirectional communication is required ([request-response](
/fundamentals/messaging-patterns/request-response)).

## Mechanism

- TODO: add diagram "Blackboard in Shared Memory"

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
