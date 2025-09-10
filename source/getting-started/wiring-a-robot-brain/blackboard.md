# Blackboard

```{admonition} Learning Objectives

TODO
```

Now that our robot Larry can reliably drive from `A` to `B`, we want to tweak
the hardware and increase the update rate of our cyclic sensor data. For
example, sending out the ultrasonic sensor distance only every 100 ms might no
longer be sufficient. Similarly, a user might want to configure the battery
threshold at which Larry raises a low-battery alarm.

As systems grow more complex, they often accumulate thousands of individual
configuration settings. Some are shared between processes, others are
process-specific. With hundreds of processes in play, managing configuration
efficiently becomes critical.

A naïve approach would be to use publish-subscribe: one process manages the
configuration and publishes updates to all subscribers. But this can quickly
turn into a memory bottleneck. Consider a 1 MB configuration shared with 1000
processes. Since iceoryx2 always pre-allocates for the worst case, it must
reserve 1001 MB of memory - one copy for the publisher and one potential copy
for each subscriber. Apart from the memory waste, you also risk configuration
inconsistencies.

The solution is the blackboard messaging pattern: a key-value repository in
shared memory. Each process can access exactly the entries it needs, such as the
ultrasonic sensor’s update rate. Any shared-memory-compatible type can be stored
in the blackboard.

Let’s implement an example where a user app can configure the sensor update
rate. A higher rate means Larry reacts to obstacles faster and can drive faster.

## Writer

As always, we start by creating a node:

```rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new().create::<ipc::Service>()?;
```

With the blackboard pattern, all key-value pairs must be defined when the
service is created. Unlike publish-subscribe, you cannot announce a key without
providing a value. That means exactly one participant is responsible for
creating the service and initializing default values. All other participants
simply open the service later.

We now create a blackboard with two settings:

* the update rate in milliseconds, and
* the battery threshold for triggering the low-battery alarm.

For keys we use the `FixedSizeByteString` type from the iceoryx2 base library.
If portability across languages is a concern, integers could be used instead.

```rust
use iceoryx2_bb_container::byte_string::FixedSizeByteString;

type KeyType = FixedSizeByteString<64>;
let service = node.service_builder(&"global_config".try_into()?)
    .blackboard_creator::<KeyType>()
    // low battery warning when load is below 25%
    .add::<f32>(&"battery_threshold".try_into()?, 0.25)
    // default ultrasonic update rate = 100 ms
    .add::<u32>(&"ultra_sonic_sensor_update_rate_in_ms".try_into()?, 100)
    .create()?;
```

Now we create a writer port to update values:

```rust
let writer = service.writer_builder().create()?;
```

To update an entry, we first obtain a handle for it. The handle is type-checked,
if you provide the wrong type, you’ll get an error instead of silent corruption.

```rust
let mut battery_threshold_handle =
    writer.entry::<f32>(&"battery_threshold".try_into()?);
let mut update_rate_handle =
    writer.entry::<u32>(&"ultra_sonic_sensor_update_rate_in_ms".try_into()?);
```

In the event loop, we periodically check for user input and update entries as
needed:

```rust
while node.wait(Duration::from_millis(100)).is_ok() {
    if let Some(new_battery_threshold) = get_battery_threshold() {
        // small value -> simple copy API
        battery_threshold_handle.update_with_copy(new_battery_threshold);
    }

    if let Some(new_update_rate) = get_update_rate() {
        // larger values -> zero-copy loan API
        let value_uninit = update_rate_handle.loan_uninit();
        let value = value_uninit.write(new_update_rate);
        // loan consumes the handle, returned when the update completes
        update_rate_handle = value.update();
    }
}
```

## Reader

On the subscriber side, we reuse the node setup from the publish-subscribe
example. We skip to opening the blackboard service:

```rust
type KeyType = FixedSizeByteString<64>;
let service = node.service_builder(&"global_config".try_into()?)
    .blackboard_opener::<KeyType>()
    .open()?;
```

Now we create a reader port:

```rust
let reader = service.reader_builder().create()?;
```

We’re interested in the `ultra_sonic_sensor_update_rate_in_ms`. We obtain a
handle that always points to the latest value:

```rust
let update_rate_handle =
    reader.entry::<u32>(&"ultra_sonic_sensor_update_rate_in_ms".try_into()?);
```

The sensor loop looks almost identical to our original publisher code, except
the update interval now comes from the global configuration:

```rust
while node.wait(Duration::from_millis(update_rate_handle.get())).is_ok() {
    let sample = publisher.loan_uninit()?;

    let sample = sample.write_payload(Distance {
        distance_in_meters: get_ultra_sonic_sensor_distance(),
        some_other_property: 42.0,
    });

    sample.send()?;
}
```

## Related Examples

<!-- markdownlint-disable-next-line MD033 -->
<br/>

````{grid} 1 1 2 2

```{grid-item}
### {octicon}`code` Rust

{octicon}`mark-github` [Minimal Creator/Writer](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/blackboard/creator.rs)  
{octicon}`mark-github` [Minimal Reader](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/blackboard/opener.rs)
```

```{grid-item}
**{octicon}`code` Python**

Not yet available  
{octicon}`mark-github` [Create pull request](https://github.com/eclipse-iceoryx/iceoryx2/pulls)
```

```{grid-item}
**{octicon}`code` C++**

Not yet available  
{octicon}`mark-github` 
[Create pull request](https://github.com/eclipse-iceoryx/iceoryx2/pulls)
```

```{grid-item}
**{octicon}`code` C**

Not yet available  
{octicon}`mark-github` 
[Create pull request](https://github.com/eclipse-iceoryx/iceoryx2/pulls)
```


````
