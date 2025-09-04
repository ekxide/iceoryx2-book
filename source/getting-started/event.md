# Event

Larry now has his distance sensor sorted out, but he’s still clumsy. We’ll add
two more sensors:

* A **battery sensor** that warns the system health monitor when juice is
  running low.
* A **bump sensor** that tattles when Larry walks into a wall.

This is where the event messaging pattern comes in. Instead of streaming values,
a process can send a one-off notification through a notifier port to another
process that is sleeping on a listener port. The listener wakes up only when
something worth caring about happens.

In our case: low battery or wall collision. The health monitor reacts by
lighting up the battery LED or going into a parking position.

Before diving into code, let’s clear up two terms:

* **State**: the current facts in memory (e.g. the battery is low or normal).
* **Event**: the notification that state changed.

If we hammer the listener with “battery low” notifications, it will still only
see one event until it wakes up and resets the flag. This prevents events from
being lost, but also means they don’t queue up like publish-subscribe messages.
Order is undefined; you only know that at least once someone signaled a state
change.

## Notifier

As usual, everything begins with a node and a service, but this time we create
an event service:

```rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new().create::<ipc::Service>()?;

let event_service = node
    .service_builder(&"system health events".try_into()?)
    .event()
    .open_or_create()?;
```

Now we create the notifier port, which sends notifications:

```rust
let notifier = event_service.notifier_builder().create()?;
```

We want to notify about two distinct things: wall collisions and low battery.
So we define two event IDs:

```rust
let wall_was_hit = EventId::new(0);
let battery_is_low = EventId::new(1);
```

Checking the sensors every second is good enough for health monitoring, so we
send notifications only when something changes:

```rust
while node.wait(Duration::from_secs(1)).is_ok() {
    if bump_sensor_was_activated() {
        notifier.notify_with_custom_event_id(wall_was_hit)?;
    }

    if battery_state() < battery_threshold {
        notifier.notify_with_custom_event_id(battery_is_low)?;
    }
}
```

[GitHub Notifier Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/event/notifier.rs)

## Listener

On the other side, the listener is the process waiting for the health events.
First,  create the node and open the event service:

```rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new().create::<ipc::Service>()?;

let event_service = node
    .service_builder(&"system health events".try_into()?)
    .event()
    .open_or_create()?;
```

Now we create the listener port:

```rust
let listener = event_service.listener_builder().create()?;
```

This process doesn’t need to poll. It just blocks until an event arrives, then
checks which one it was:

```rust
while listener.blocking_wait_all(|event_id| {
    if event_id == battery_is_low {
        activate_battery_warning_light();
    }
    if event_id == wall_was_hit {
        go_into_parking_position();
    }
}).is_ok() {}
```

[GitHub Listener Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/event/listener.rs)
