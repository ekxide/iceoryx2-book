# Publish–Subscribe

Larry’s hardware and algorithms are already built. Now we want to send the
distance measured by his ultrasonic sensor to another process that can slam the
brakes if an obstacle gets closer than X meters.

In iceoryx2, that’s a perfect job for the **publish–subscribe** pattern: one
process publishes a stream of distances, another subscribes and reacts.

```lua
+--------------------+           +-------------------+
|  Ultrasonic Sensor |           |  Emergency Brake  |
|   (Publisher)      |           |    (Subscriber)   |
+--------------------+           +-------------------+
          |                                ^
          |     "distance_to_obstacle"     |
          +------------[ Service ]---------+
```

## Publisher

Everything in iceoryx2 starts with a node. A node represents a communication
point (like a process or thread) and acts as a factory for services. To keep
things sane when debugging later, we’ll give this node a name:

```rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new()
    .name(&"UltraSonicSensor".try_into()?)
    .create::<ipc::Service>()?;
```

Now we can create a service called `"distance_to_obstacle"`, using a struct
`Distance` as the payload type. Payloads must be shared-memory compatible,
meaning:

* self-contained
* no internal pointers
* identical memory representation in every process

```rust
// every payload must implement ZeroCopySend
#[derive(ZeroCopySend)]
#[repr(C)] // ensures identical layout across processes
pub struct Distance {
    pub distance_in_meters: f64,
    pub some_other_property: f32,
}
```

With the payload defined, we can set up the service:

```rust
let service = node
    .service_builder(&"distance_to_obstacle".try_into()?)
    .publish_subscribe::<Distance>()
    .open_or_create()?;
```

Services also provide introspection. For example, you can query how many
subscribers are currently connected:

```rust
service.dynamic_config().number_of_subscribers();
```

For now, though, we just want a publisher:

```rust
let publisher = service.publisher_builder().create()?;
```

Larry isn’t exactly Formula 1 material, so publishing every 100 ms is plenty. We
set up a loop, wait 100 ms, read the sensor, and send the data:

```rust
while node.wait(Duration::from_millis(100)).is_ok() {
    // acquire sensor reading and send it
}
```

To fully benefit from zero-copy, we don’t allocate or clone data. Instead, we
loan an uninitialized sample from the publisher’s memory pool, fill it, and then
send it:

```rust
let sample = publisher.loan_uninit()?;

let sample = sample.write_payload(Distance {
    distance_in_meters: get_ultra_sonic_sensor_distance(),
    some_other_property: 42.0,
});

sample.send()?;
```

Whenever we acquire an uninitialized sample, we must write the payload to it and
convert it into an initialized sample. This ensures we don’t accidentally ship
uninitialized garbage across processes. Once the payload is written, the sample
is safe to send.

[GitHub Publisher-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/publish_subscribe/publisher.rs)

## Subscriber

The subscriber setup starts the same: create a node, open the
`"distance_to_obstacle"` service, and specify the payload type.

```rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new().create::<ipc::Service>()?;

let service = node
    .service_builder(&"distance_to_obstacle".try_into()?)
    .publish_subscribe::<Distance>()
    .open_or_create()?;
```

Now we create the subscriber side:

```rust
let subscriber = service.subscriber_builder().create()?;
```

Since the publisher sends updates every 100 ms, we loop at the same pace and
check for new data. If we receive something, we print it:

```rust
while node.wait(Duration::from_millis(100)).is_ok() {
    while let Some(sample) = subscriber.receive()? {
        println!("received distance {:?}", sample.payload());
    }
}
```

From here, reacting is easy: if the distance falls below a threshold, Larry can
hit the emergency brake before smashing into a wall.

[GitHub Subscriber-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/publish_subscribe/subscriber.rs)
