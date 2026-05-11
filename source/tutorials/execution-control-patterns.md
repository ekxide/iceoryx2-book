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
process and wake-up signals are emitted automatically when data arrives. While
convenient, this coupling can introduce unwanted latency through system calls
and context switches.

`iceoryx2` instead decouples control flow from data flow, so the user
decides when a participant thread is put to sleep and when it is made
ready to run. This lets the user pick execution patterns suited to the
application's requirements: minimal latency, efficient CPU usage, or
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

### Example: Market data consumer

A trading process that consumes a stream of price ticks from a market data
feed has no fixed schedule. Ticks arrive whenever the market sends them,
and the consumer must react with the smallest possible delay.

```rust
use core::time::Duration;
use iceoryx2::prelude::*;

const TARGET_PRICE: f64 = 100.0;

#[derive(Debug, Clone, Copy, ZeroCopySend)]
#[repr(C)]
struct PriceTick {
    instrument_id: u32,
    price: f64,
}

let node = NodeBuilder::new()
    .name(&"MarketDataConsumer".try_into()?)
    .create::<ipc::Service>()?;

let ticks_service = node
    .service_builder(&"market/ticks".try_into()?)
    .publish_subscribe::<PriceTick>()
    .open_or_create()?;

let orders_service = node
    .service_builder(&"market/orders".try_into()?)
    .publish_subscribe::<u32>()
    .open_or_create()?;

let subscriber = ticks_service.subscriber_builder().create()?;
let orders_publisher = orders_service.publisher_builder().create()?;

while node.wait(Duration::ZERO).is_ok() {
    while let Some(tick) = subscriber.receive()? {
        if tick.price > TARGET_PRICE {
            orders_publisher.send_copy(tick.instrument_id)?;
        }
    }
}
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

### Example: Cruise control

A cruise controller reads the vehicle's current speed, computes a throttle
command, and sends it to the actuator at a fixed rate. The control math
depends on a stable loop period.

```rust
use core::time::Duration;
use iceoryx2::prelude::*;

const CONTROL_PERIOD: Duration = Duration::from_millis(20); // 50 Hz
const SETPOINT_KMH: f32 = 100.0;
const KP: f32 = 0.1;

let node = NodeBuilder::new()
    .name(&"CruiseController".try_into()?)
    .create::<ipc::Service>()?;

let speed_service = node
    .service_builder(&"vehicle/speed".try_into()?)
    .publish_subscribe::<f32>()
    .subscriber_max_buffer_size(1)
    .open_or_create()?;

let throttle_service = node
    .service_builder(&"vehicle/throttle".try_into()?)
    .publish_subscribe::<f32>()
    .open_or_create()?;

let speed_subscriber = speed_service.subscriber_builder().create()?;
let throttle_publisher = throttle_service.publisher_builder().create()?;

let mut current_speed = 0.0f32;

while node.wait(CONTROL_PERIOD).is_ok() {
    match speed_subscriber.receive() {
        Ok(Some(sample)) => current_speed = *sample,
        Ok(None) => { /* no new sample — reuse last known speed */ }
        Err(_) => { /* TODO: handle receive failure */ }
    }

    let throttle = (KP * (SETPOINT_KMH - current_speed)).clamp(0.0, 1.0);
    throttle_publisher.send_copy(throttle)?;
}
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

Each notification carries an `EventId`. Notifiers and listeners are
many-to-many on a service, so the same `EventId` from several producers can
wake one listener (for example, a pool of workers all flagging "data ready").  
Conversely, a single listener can branch on the `EventId` to handle different
kinds of work — shutdown, config change, frame ready — without spinning
up extra services.

This approach combines efficient CPU usage with timely response when work
arrives: threads wake only when there is work to do.

Events are pure signals; they carry no payload. To move data alongside a
wake-up, pair an event service with a publish-subscribe service: the
producer publishes a sample, then notifies; the consumer wakes and drains
its inbox. This reconstructs the data-plus-wake-up flow that other
middleware couples implicitly, while keeping the data path and the control
path as separate, independently tunable services.

Multiple notifications can also coalesce into a single wake-up, so the
listener should drain all pending events on each wake rather than handle
just one.

### Example: Frame processor

A processor that consumes camera frames from an upstream component shouldn't
busy-poll. Frames arrive periodically (or in bursts when the camera
buffers a few), and the processor has nothing useful to do between them.
Pairing a publish-subscribe service for the frames with an event service
for "new frame ready" notifications lets the processor sleep until a frame
is available.

```rust
use core::time::Duration;
use iceoryx2::prelude::*;

#[derive(Debug, Clone, Copy, ZeroCopySend)]
#[repr(C)]
struct CameraFrame {
    frame_id: u64,
    width: u32,
    height: u32,
    timestamp_ns: u64,
}

let node = NodeBuilder::new()
    .name(&"FrameProcessor".try_into()?)
    .create::<ipc::Service>()?;

let frames_service = node
    .service_builder(&"camera/frames".try_into()?)
    .publish_subscribe::<CameraFrame>()
    .open_or_create()?;

let signal_service = node
    .service_builder(&"camera/frames".try_into()?)
    .event()
    .open_or_create()?;

let subscriber = frames_service.subscriber_builder().create()?;
let listener = signal_service.listener_builder().create()?;

while node.wait(Duration::ZERO).is_ok() {
    if listener.timed_wait_one(Duration::from_secs(1))?.is_some() {
        while let Some(frame) = subscriber.receive()? {
            // process frame
        }
    }
}
```

The thread sleeps inside `timed_wait_one` until a notification arrives or
one second elapses, whichever comes first. The one-second timeout is a
safety net for the outer `node.wait(Duration::ZERO)` signal check; the
real wake-up trigger is the notification. When the listener wakes, the
subscriber drain handles whatever frames have accumulated.

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
notification and deadline attachments. Drain all pending events inside
the callback, otherwise the `WaitSet` will keep waking the thread as long
as there are queued events on a source.

### Example: Vehicle supervisor

A vehicle supervisor pulls together three concerns into one thread: a
periodic control step, a sensor whose silence is itself a signal, and an
emergency-stop input that can interrupt either of them. Multiplexing these
on a single `WaitSet` lets the supervisor sleep between events and react
to whichever one fires next.

```rust
use core::time::Duration;
use iceoryx2::prelude::*;

const CONTROL_PERIOD: Duration = Duration::from_millis(20); // 50 Hz
const SPEED_DEADLINE: Duration = Duration::from_millis(100);

let node = NodeBuilder::new()
    .name(&"VehicleSupervisor".try_into()?)
    .create::<ipc::Service>()?;

let speed_signal = node
    .service_builder(&"vehicle/speed".try_into()?)
    .event()
    .open_or_create()?;

let emergency_stop_signal = node
    .service_builder(&"vehicle/emergency_stop".try_into()?)
    .event()
    .open_or_create()?;

let speed_listener = speed_signal.listener_builder().create()?;
let emergency_stop_listener = emergency_stop_signal.listener_builder().create()?;

let waitset = WaitSetBuilder::new().create::<ipc::Service>()?;
let tick_guard = waitset.attach_interval(CONTROL_PERIOD)?;
let speed_guard = waitset.attach_deadline(&speed_listener, SPEED_DEADLINE)?;
let emergency_stop_guard = waitset.attach_notification(&emergency_stop_listener)?;

waitset.wait_and_process(|attachment_id| {
    if attachment_id.has_event_from(&tick_guard) {
        // run periodic control step
    }
    if attachment_id.has_missed_deadline(&speed_guard) {
        // speed sensor went silent — engage fail-safe
    }
    if attachment_id.has_event_from(&speed_guard) {
        speed_listener
            .try_wait_all(|_id| { /* drain */ })
            .unwrap();
        // process new speed reading
    }
    if attachment_id.has_event_from(&emergency_stop_guard) {
        emergency_stop_listener
            .try_wait_all(|_id| { /* drain */ })
            .unwrap();
        // engage emergency stop
    }
    CallbackProgression::Continue
})?;
```

Each attachment type maps to one of the patterns covered earlier:
`attach_interval` is the control-step heartbeat, `attach_notification`
is the emergency-stop wake-up, and `attach_deadline` combines both.
The supervisor receives ordinary speed updates as notifications, and
the deadline fires only if the speed sensor goes silent past the budget.
Inside each notification branch, `try_wait_all` drains any coalesced
events on that listener before the callback returns.

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
