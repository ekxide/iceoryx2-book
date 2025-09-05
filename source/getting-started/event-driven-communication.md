# Event-Driven Communication

Before jumping into the example, we need to clarify the difference between
**data flow** and **control flow**.

In programming, control flow is about *when* a function executes, while data
flow is about *what inputs* it receives. In iceoryx2 these two concepts are
separated on purpose, unlike in many network protocols where receiving data
automatically involves a syscall that wakes up the process.

Messaging patterns like publish–subscribe and request–response define the data
flow. The event messaging pattern handles control flow. Why split them? Because
you don’t always want to pass data when triggering execution. Sometimes you just
need to wake up a function with no arguments. Other times you need several
arguments, and you don’t want to be interrupted until all of them have arrived.
Mixing data and control flow forces you into hacks like sending empty messages
or waking up too early.

By keeping them separate, iceoryx2 gives you explicit control over *when* your
system reacts.

## Why It Matters

Take the emergency brake system of our robot Larry. It doesn’t need every single
distance sample from the ultrasonic sensor. If the next obstacle is basically on
another planet, we can ignore it. But when an obstacle is close, we suddenly
care. And not just about the raw number but also the trend over time. If the
object is receding, no action is needed; if it’s approaching, we may need to hit
the brakes.

Now consider failure cases. If the sensor process dies and never sends another
update, Larry shouldn’t drive happily into oblivion. A safe fallback (parking,
for example) is required. Another twist: the emergency brake process might start
after the sensor process. In that case, it still needs the most recent three
distance samples to compute not just position, but relative speed and
acceleration.

These scenarios show why we want both streams of data and independent control
signals.

## Publisher Setup

We start by creating a node and defining two services with the same name:

* a publish–subscribe service for distance samples,
* an event service for control signals.

The subscriber needs the last three samples whenever it connects, so we
configure both the subscriber buffer and history size accordingly. The
subscriber also needs to hold on to/borrow three samples in parallel to compute
position, speed, and acceleration.

```rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new()
    .create::<ipc::Service>()?;

let pubsub_service = node
    .service_builder(&"distance_to_obstacle".try_into()?)
    .publish_subscribe::<Distance>()
    .subscriber_max_buffer_size(3)
    .history_size(3)
    .subscriber_max_borrowed_samples(3)
    .open_or_create()?;
```

For the event service, we configure a special event that fires if the sensor
process is identified as dead, allowing the emergency brake to switch to a safe
state.

```rust
let ultra_sonic_service_dead = EventId::new(10);
let event_service = node
    .service_builder(&"distance_to_obstacle".try_into()?)
    .event()
    .notifier_dead_event(ultra_sonic_service_dead)
    .open_or_create()?;
```

Now we create a publisher for the distance samples and a notifier for control
events.

```rust
let publisher = pubsub_service.publisher_builder().create()?;
let notifier = event_service.notifier_builder().create()?;

let obstacle_too_close = EventId::new(5);
```

The publishing loop: send the distance sample every 100 ms, and trigger an event
if it’s below the threshold.

```rust
while node.wait(Duration::from_millis(100)).is_ok() {
    let sample = publisher.loan_uninit()?;

    let distance = get_ultra_sonic_sensor_distance();
    let sample = sample.write_payload(Distance {
        distance_in_meters: distance,
        some_other_property: 42.0,
    });

    sample.send()?;

    if distance < distance_threshold {
        notifier.notify_with_custom_event_id(obstacle_too_close)?;
    }
}
```

[GitHub Event-Driven Publisher Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/event_based_communication/publisher.rs)

## Subscriber Setup

On the other side, we again create both services and their ports:

```rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new()
    .create::<ipc::Service>()?;

let pubsub_service = node
    .service_builder(&"distance_to_obstacle".try_into()?)
    .publish_subscribe::<Distance>()
    .subscriber_max_buffer_size(3)
    .history_size(3)
    .subscriber_max_borrowed_samples(3)
    .open_or_create()?;

let event_service = node
    .service_builder(&"distance_to_obstacle".try_into()?)
    .event()
    .notifier_dead_event(EventId::new(10))
    .open_or_create()?;

let subscriber = pubsub_service.subscriber_builder().create()?;
let listener = event_service.listener_builder().create()?;
```

Instead of polling every 100 ms, the subscriber just waits for events. When
woken up, it processes them and goes back to sleep.

```rust
while listener.blocking_wait_all(|event_id| {
    if event_id == ultra_sonic_service_dead {
        go_into_parking_position();
    }

    if event_id == obstacle_too_close {
        let mut last_samples = vec![];
        while let Some(sample) = subscriber.receive()? {
            last_samples.push(sample);
            if last_samples.len() == 3 {
                break;
            }
        }
        perform_break(last_samples);
    }
}).is_ok() {}
```

[GitHub Event-Driven Subscriber Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/event_based_communication/subscriber.rs)

## Health Monitoring

The notifier’s “dead event” relies on health monitoring: iceoryx2 offers
building blocks to detects when a process dies. How this works in detail is
covered in a separate tutorial.
