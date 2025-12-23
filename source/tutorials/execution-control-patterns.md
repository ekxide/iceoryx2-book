# Execution control patterns

```{admonition} Learning Objectives
This tutorial explores four common patterns for controlling execution flow,
each with different trade-offs:

1. Busy looping for minimal latency at the cost of CPU usage
2. Timer-based sleep and wake-up for periodic work
3. Event-driven sleep and wake-up for reactive applications
4. Multiplexing to wait on multiple event sources simultaneously
```

`iceoryx2` decouples control flow from data flow, giving you explicit control
over when your participants wait and run. Many middleware systems
automatically put threads to sleep when waiting for data and emit wake-up
events when samples are sent. While convenient, this coupling can introduce
unwanted latency through unnecessary context switches and wake-ups when no
actual work is required.

By separating these concerns, `iceoryx2` lets you choose execution patterns
optimized for your specific requirements—whether minimizing latency, reducing
CPU usage, or handling complex event scenarios. You can even mix patterns
within a single application, using the approach that best fits each
participant's needs.

## Busy looping

The simplest execution control pattern continuously polls for new data without
ever sleeping. This approach minimizes latency by eliminating context switches
and system calls, making it ideal for ultra-low-latency scenarios where CPU
usage is not a constraint.

In a busy loop, a participant repeatedly checks for available samples in
a tight loop. While this consumes an entire CPU core, it guarantees the fastest
possible response time when data arrives.

```rust
TODO
```

Busy looping is most appropriate for time-critical paths where achieving the
lowest possible latency is the priority and you have dedicated CPU cores
available.

## Periodic execution with timers

Periodic execution uses timers to schedule work at regular intervals, allowing
a participant to sleep between executions enabled your system to handle
other tasks. This pattern is ideal for activities that run on a fixed schedule,
such as sensor polling, status updates, heartbeat mechanisms, or other periodic
data processing.

With timer-based execution, a participant explicitly requests the platform to
wake it after a specified duration. This approach significantly reduces CPU
usage compared to busy looping.

```rust
TODO
```

The trade-off is that wake-up timing depends on the platform's scheduler
performance characteristics and the system load.

## Reactive execution with events

Reactive execution allows a participant to sleep until a specific event
occurs, waking only when there is relevant work to perform. This pattern
enables efficient event-driven systems that respond to asynchronous stimuli
without wasting CPU cycles on polling or being delayed by a fixed execution
schedule.

`iceoryx2` implements this through the [event messaging pattern](
/fundamentals/messaging-patterns/event). Participants wait on listener ports,
which put their threads to sleep until events are received from notifiers
attached to the service.

```rust
TODO
```

When an event occurs, the participant's thread is scheduled to run, allowing
it to respond as soon as the platform is able to schedule it to run. This
provides both CPU efficiency during idle periods and responsive behavior when
events arrive at the cost of latency introduced by system calls
and context switches when transitioning between sleep and wake states.

## Multiplexed event handling

Multiplexed event handling enables a single participant to wait on multiple
event sources simultaneously, waking when any of them fires.

`iceoryx2` provides a `WaitSet` that allows you to register multiple timers
and listeners and block until any registered event occurs. When any event
source fires, the participant wakes and can determine which source triggered
it and process accordingly.

```rust
TODO
```

Multiplexed event handling provides the efficiency of reactive execution while
handling multiple event sources in a single participant. Like reactive
execution, it incurs latency from system calls and context switches when
transitioning between sleep and wake states, though the behavior may differ
when multiple sources are registered.

The additional trade-off is increased complexity in managing the `WaitSet` and
determining which event source triggered the wake-up, compared to simpler
patterns that wait on a single event source.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Understand the Event messaging pattern
:link: /fundamentals/messaging-patterns/event
:link-type: doc
:shadow: none

Understand the `Event` messaging pattern.
```

```{grid-item-card} Understand the WaitSet
:link: /fundamentals/execution-control
:link-type: doc
:shadow: none

Understand the `WaitSet` component.
```

```{grid-item-card} Example: Event Multiplexing (Rust)
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/rust/event_multiplexing
:link-type: url

See how to handle multiple `Event` services in a single user thread.
```

````
