# Execution Control Patterns

```{admonition} Learning Objectives
This tutorial explores four common patterns for controlling execution flow,
each with different trade-offs:

1. Busy looping for minimal latency, at the cost of a saturated CPU core
2. Timer-driven wake-ups for periodic work, at the cost of responsiveness between ticks
3. Event-driven wake-ups for responsiveness and efficient CPU usage, at the cost of OS wake-up latency
4. Multiplexing multiple approaches in one thread
```

A pattern often seen in middleware solutions is to couple data flow with
control flow. Threads are put to sleep when no data is available to
process, and wake-up signals are emitted automatically when data
arrives.

This coupling forces a wake-up per sample. Workloads whose useful unit
of work spans multiple samples pay the cost of system calls and context
switches on wake-ups that produce no work. Examples include sensor fusion
algorithms and algorithms that operate over a window of data.

`iceoryx2` instead decouples control flow from data flow. The user
decides when a participant thread is put to sleep, when it is made
ready to run, and what signals trigger the transition.

The following sections cover the patterns for managing application
execution with `iceoryx2`.

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

### Example: Market Data Consumer

A trading process that consumes a stream of price ticks from a market data
feed has no fixed schedule. Ticks arrive whenever the market sends them,
and the consumer must react with the smallest possible delay.

```{literalinclude} ../../snippets/execution-control-patterns/src/bin/market_data_consumer.rs
:language: rust
:start-after: // snippet:start
:end-before: // snippet:end
:dedent:
```

The outer `node.wait(Duration::ZERO)` returns immediately and only errors
on `SIGTERM`/`SIGINT`, keeping the loop tight while still allowing a clean
shutdown. Each queued tick is drained inline and checked against the target
price before the next poll, so the order sees only the busy-loop's own
polling overhead, with no OS wake-up. Thread pinning and scheduling priority
are configured outside `iceoryx2`, typically at thread-spawn time.

## Timer-driven

Timer-driven execution puts threads to sleep after a unit of work is done and
reschedules them on a regular interval.

This approach limits the overhead from context switches and system calls to
the timer events. Keep in mind that the precision of timer events is
governed by the platform on which `iceoryx2` is deployed.

This pattern is well-suited to tasks that have a well-defined schedule, such as
heartbeat mechanisms or time-triggered architectures.

### Example: Cruise Control

This cruise control application reads the vehicle's current speed, computes a
throttle command, and sends it to the actuator at a fixed rate. The control
math depends on a stable loop period.

```{literalinclude} ../../snippets/execution-control-patterns/src/bin/cruise_control.rs
:language: rust
:start-after: // snippet:start
:end-before: // snippet:end
:dedent:
```

`Node::wait(CONTROL_PERIOD)` paces the loop; nothing else triggers a tick.
The speed service is configured with `subscriber_max_buffer_size(1)`, so the
queue only ever holds the latest reading; a single `receive()` per tick
fetches it. A throttle command goes out unconditionally. If no new speed
sample arrived, the controller reuses the last known value.

## Event-driven

Like timer-driven execution, event-driven execution puts threads to sleep
after a unit of work is done. The difference is that threads are woken up
in response to notifications via the [event messaging pattern](
/fundamentals/messaging-patterns/event). Events are defined by the user
and can be triggered from anywhere in the system.

Notifiers and listeners are many-to-many on a service, so any notifier
can trigger any listener attached to that service.
Additionally, the `EventId`, included with every notification, can be used
to disambiguate notifications on a single service.

This approach combines efficient CPU usage with timely response when work
arrives: threads wake only when there is work to do.

Events carry no payload beyond the `EventId`, so to move data alongside
a wake-up, an event service must be paired with a publish-subscribe
service: the producer publishes a sample, then notifies; the consumer
wakes and drains available samples.
This approach reconstructs the data-plus-wake-up flow that other middlewares
couple implicitly.

### Example: Event Data Recorder

A vehicle event-data recorder packages and uploads sensor data to the cloud
whenever an incident occurs (for instance, when the emergency brake fires)
so the scenario can be replayed for analysis. Sensors publish continuously
on their own schedules; the recorder only has work to do at incident time.

The event-data recorder does not need to wake on publish of every sample.
This would introduce overhead when no real work needs to be done. Instead,
a single event service signals when the work should be done.

```{literalinclude} ../../snippets/execution-control-patterns/src/bin/event_data_recorder.rs
:language: rust
:start-after: // snippet:start
:end-before: // snippet:end
:dedent:
```

Each pubsub service is configured with a large `subscriber_max_buffer_size`,
so the recorder accumulates a lookback window without dropping samples
between triggers. When a trigger arrives, `try_wait_all` drains any
further triggers queued behind it, so several sensors signalling the
same incident produce one upload rather than several near-empty ones.
Draining each subscriber to exhaustion then captures the full pre-event
window. The one-second timeout on `timed_wait_one` ensures the outer
`node.wait(Duration::ZERO)` runs at least once per second, so the
recorder can notice `SIGTERM`/`SIGINT` and shut down cleanly even when
no triggers are arriving.

## Multiplexing

Multiplexing combines several of the patterns above into a single thread.
`iceoryx2` provides a `WaitSet` that holds a set of attachments: interval
timers, event notifications, and deadline-monitored event sources. The
execution of the thread can be handed over to the `WaitSet`, which
puts the thread to sleep and wakes it up when any attachment is triggered.
When the wake-up arrives, the `WaitSet` invokes a user-supplied callback
identifying which attachment triggered the wake-up.

A `WaitSet` supports three kinds of attachments:

* `attach_interval(duration)` — a periodic tick, equivalent to the
  timer-driven pattern but composable with other sources.
* `attach_notification(&listener)` — wake on any event from the attached
  listener, equivalent to the event-driven pattern.
* `attach_deadline(&listener, duration)` — wake on an event _or_ if no
  event arrives within the deadline. The deadline resets every time an
  event is received e.g. fail safe if no sensor sample arrives within
  100 ms.

Multiplexing keeps the efficient sleep/wake behaviour of the single-source
patterns while consolidating several concerns into one thread at the cost of
bookkeeping complexity, the callback has to disambiguate which source fired
before handling it.

### Example: Vehicle supervisor

A vehicle supervisor pulls together three concerns into one thread: a
periodic control step, a sensor whose silence is itself a signal, and an
emergency-stop input that can interrupt either of them. Multiplexing these
on a single `WaitSet` lets the supervisor sleep between events and react
to whichever one fires next.

```{literalinclude} ../../snippets/execution-control-patterns/src/bin/vehicle_supervisor.rs
:language: rust
:start-after: // snippet:start
:end-before: // snippet:end
:dedent:
```

Each attachment type maps to one of the patterns covered earlier:
`attach_interval` is the control-step heartbeat, `attach_notification`
is the emergency-stop wake-up, and `attach_deadline` combines both.
The supervisor receives ordinary speed updates as notifications, and
the deadline fires only if the speed sensor goes silent past the budget.
Each notification branch must drain its listener with `try_wait_all`
before returning. While unread notifications remain, the `WaitSet`
treats the source as still ready and re-enters the callback
immediately, so a branch that doesn't drain spins the thread.

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

```{grid-item-card} Code Examples
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples
:link-type: url

Explore some additional `iceoryx2` usage patterns.
```

````
