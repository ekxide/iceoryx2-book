# Execution Control

Developers have complete control over the execution of their applications with
`iceoryx2`.

There are no background threads; all house-keeping operations are carried out
in the application threads. They occur either through usual interaction with
the API (opening services, sending or receiving data, etc.) which provides a
frictionless default user experience, or through deliberate calls made to
house-keeping functions when more control is required.

This control over execution is essential for safety-critical contexts where
determinism is paramount. It enables predictable resource usage, precise timing
control, and lends itself to real-time analysis and certification processes.

## Waitset

The `Waitset` is an optional component that facilitates the setup of reactive
processing loops in user threads.

```{figure} /images/execution-control-waitset.svg
:alt: execution control via waitset
:align: center
:name: fig-execution-control-with-waitset

Event Multiplexing via Waitset
```

Various event sources (depending on the platform) can be attached to the
`Waitset`, which takes care of putting the thread to sleep and waking it up
when events occur on any of them. This functionality is achieved using the
available event multiplexing facilities of the platform.

This approach conserves CPU while maintaining responsive event processing,
however, the sleep-wake-up behaviour may introduce additional latency
depending on the implementation of the platform's event multiplexing
facilities.

Possible event sources that can be attached to a `Waitset` could include:

1. Interval timers
2. Deadline timers
3. [`Listener` ports](messaging-patterns/event.md)
4. File-descriptor-based event sources (on compatible platforms)

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Understand the Communication Model
:link: /fundamentals/communication-model
:link-type: doc
:shadow: none

Get familiar with the components involved with establishing communication.
```

```{grid-item-card} Example: Event Signalling
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/rust/event
:link-type: url
:shadow: none

See how to coordinate participants with events.
```

```{grid-item-card} Example: Event Multiplexing (Rust)
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/rust/event_multiplexing
:link-type: url
:shadow: none

See how to use the `WaitSet` in your application.
```

````
