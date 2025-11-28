# Event

```{figure} /images/event-pattern.svg
:alt: event messaging pattern
:align: center
:name: fig-event-communication

Event Communication Model
```

A messaging pattern for coordinating execution between participants. The
`Event` messaging pattern is not intended for sharing payload data, but
rather communicating to other participants that there is work to do.

## Mechanism

```{figure} /images/event-mechanism.svg
:alt: event messaging pattern
:align: center
:name: fig-event-mechanism

Events over Native Sleep/Wake-up Mechanisms
```

The event messaging pattern abstracts over sleep and wake-up
mechanisms of the platform.
Listeners can put their thread into a sleep state and be woken up by notifiers
on the service.

A single participant can handle multiple listeners
[using a `Waitset`](../execution-control.md#waitset), which allows the
participant to wait on several event sources simultaneously.

An `EventId` is provided with each notification, either assigned automatically
or specified by the notifier, to facilitate event handling by the receiving
participants.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Understand Execution Control
:link: /fundamentals/execution-control
:link-type: doc

Understand how `Event` services fit into the bigger picture of execution control
with `iceoryx2`.
```

```{grid-item-card} Example: Event Service (Rust)
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/rust/event
:link-type: url

See how to set up a basic `Event` service.
```

```{grid-item-card} Example: Event Multiplexing (Rust)
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/rust/event_multiplexing
:link-type: url

See how to handle multiple `Event` services in a single user thread.
```

````
