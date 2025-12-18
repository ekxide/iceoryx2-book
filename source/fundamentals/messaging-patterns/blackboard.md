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
readers are interested in a subset of the key-value pairs, such as global
configuration settings adjusted at runtime or a regularly updated global state.

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

The blackboard messaging pattern currently allows one writer and multiple
readers. These can obtain write or read permission for specific key-value
pairs. The writer is then able to update the respective values efficiently,
while the readers can efficiently read the latest values - without having to
iterate the complete key-value-repository again.

Each key-value entry has a unique ID which can be sent along with events so
listening participants are able to react to specific updates in the blackboard.

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
