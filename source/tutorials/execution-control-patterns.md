# Execution control patterns

```{admonition} Learning Objectives
This tutorial explores four common patterns for controlling execution flow,
each with different trade-offs:

1. Busy looping for minimal latency at the cost of CPU usage
2. Timer-driven sleep and wake-up for periodic work
3. Event-driven sleep and wake-up for reactive applications
4. Multiplexing to wait on multiple sources simultaneously
```

A pattern often seen in middleware solutions is to couple data flow with
control flow. Threads are put to sleep when no data is available to
process and wake-up signals are emitted automatically when data arrives. While
convenient, this coupling can introduce unwanted latency through system calls
and context switches.

`iceoryx2` instead decouples control flow from data flow, so the user
decides when a participant thread is put to sleep and when it is made
ready to run. This lets the user pick execution patterns suited to the
application's requirements — minimal latency, efficient CPU usage, or
coordination across multiple sources.

## Busy looping

Busy looping continuously polls ports for new data without sleeping the
thread. This approach minimizes latency by eliminating context switches
and system calls, making it ideal for ultra-low-latency scenarios where
efficient CPU usage is not a priority.

A busy loop saturates one CPU core for as long as it runs, so it fits
hot-path consumers on isolated cores, such as high-frequency trading and
kernel-bypass networking, where the latency budget justifies dedicating
the core. Practical deployments pin the polling thread to a specific core
and raise its scheduling priority.

```rust
TODO
```

## Timer-driven

Timer-driven execution puts threads to sleep after a unit of work is done and
reschedules them on a regular interval.

This approach limits the overhead from context switches and system calls to
the timer events. Keep in mind that the precision of timer events is
governed by the platform on which `iceoryx2` is deployed.

This pattern is well-suited to tasks that have a well-defined schedule, such as
heartbeat mechanisms or time-triggered architectures.

```rust
TODO
```

## Event-driven

Like timer-driven execution, event-driven execution puts threads to sleep
after a unit of work is done. The difference is that threads are woken up
in response to notifications via the [event messaging pattern](
/fundamentals/messaging-patterns/event). Events are defined by the user
and can be triggered from anywhere in the system.

Each notification carries an `EventId`. Notifiers and listeners are
many-to-many on a service, so the same `EventId` from several producers can
wake one listener — for example, a pool of workers all flagging "data ready".  
Conversely, a single listener can branch on the `EventId` to handle different
kinds of work — shutdown, config change, frame ready — without spinning
up extra services.

This approach combines efficient CPU usage — threads wake only when there
is work to do — with timely response when work arrives.

Events are pure signals — they carry no payload. To move data alongside a
wake-up, pair an event service with a publish-subscribe service: the
producer publishes a sample, then notifies; the consumer wakes and drains
its inbox. This reconstructs the data-plus-wake-up flow that other
middleware couples implicitly, while keeping the data path and the control
path as separate, independently tunable services.

Multiple notifications can also coalesce into a single wake-up, so the
listener should drain all pending events on each wake rather than handle
just one.

```rust
TODO
```

## Multiplexing

Multiplexing combines several of the patterns above into a single thread.
`iceoryx2` provides a `WaitSet` that holds a set of attachments — interval
timers, event notifications, and deadline-monitored event sources — and
puts the thread to sleep until any of them fires. When the wake-up arrives,
the `WaitSet` invokes a user-supplied callback identifying which attachment
was responsible.

A `WaitSet` supports three kinds of attachments:

* `attach_interval(duration)` — a periodic tick, equivalent to the
  timer-driven pattern but composable with other sources.
* `attach_notification(&listener)` — wake on any event from the attached
  listener, equivalent to the event-driven pattern.
* `attach_deadline(&listener, duration)` — wake on an event _or_ if no
  event arrives within the deadline. The deadline resets every time an
  event is received — e.g. fail safe if no sensor sample arrives within
  100 ms.

Multiplexing keeps the efficient sleep/wake behaviour of the single-source
patterns while consolidating several concerns into one thread at the cost of
bookkeeping: each attachment returns a guard that must outlive its use, and
the callback has to disambiguate which source fired before handling it.

The same coalescing caveat as the event-driven pattern applies to
notification and deadline attachments — drain all pending events inside
the callback, otherwise the `WaitSet` will keep waking the thread as long
as there are queued events on a source.

```rust
TODO
```

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
