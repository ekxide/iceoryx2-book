# Blackboard

Now that our robot Larry can reliably drive from `A` to `B`, we want to tweak
the hardware and increase the update rate of our cyclic sensor data. For
example, we might decide that only sending out the ultrasonic sensor distance
every 100 ms might no longer be sufficient. Similarly, a user might want
to have the capability to configure the battery threshold at which Larry raises
a low-battery alarm.

As systems grow more complex, they often accumulate thousands of individual
configuration settings such as these. Some are shared between participants,
others are participant-specific. With hundreds of participants in play,
managing the configuration efficiently becomes critical.

A naïve approach would be to use publish-subscribe: where one participant
manages the configuration and publishes updates to all subscribing
participants, but this can quickly turn into a memory bottleneck. Consider a
1 MB configuration shared with 1000 participants. Since `iceoryx2` always
pre-allocates memory for the worst case, it must reserve 1001 MB of memory -
one copy for the publisher and one potential copy for each subscriber. Apart
from the memory waste, there is also the risk of configuration inconsistencies.

The solution is the blackboard messaging pattern: a key-value repository in
shared memory. Each participant can access exactly the entries it needs, such
as the ultrasonic sensor’s update rate. Any shared-memory-compatible and
trivially-copyable type, can be stored in the blackboard.

Let’s implement an example where a user app can configure the sensor update
rate. A higher rate means Larry reacts to obstacles faster and can drive faster.

## Writer

As always, we start by creating a node:

````{tab-set-code}
```{code-block} rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new().create::<ipc::Service>()?;
```

```{code-block} python
import iceoryx2 as iox2

node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)
```

```{code-block} c++
#include "iceoryx2.hpp"

using namespace iox2;

auto node = NodeBuilder().create<ServiceType::Ipc>().expect("");
```

```{code-block} c
#include "iox2/iceoryx2.h"

iox2_node_builder_h node_builder = iox2_node_builder_new(NULL);
iox2_node_h node = NULL;
if (iox2_node_builder_create(node_builder, NULL, iox2_service_type_e_IPC, &node) != IOX2_OK) {
    printf("Could not create node!\n");
    exit(-1);
}
```
````

With the blackboard pattern, all key-value pairs must be defined when the
service is created. Unlike publish-subscribe, you cannot announce a key without
providing a value. That means exactly one participant is responsible for
creating the service and initializing default values. All other participants
may open the service later.

We now create a blackboard with two settings:

* the update rate in milliseconds, and
* the battery threshold for triggering the low-battery alarm.

For keys we use the `StaticString` type from the `iceoryx2` base library.
Currently, only Rust and C++ can share the `StaticString` in-memory. If the
blackboard is also to be used in C or Python, integers can be used instead.

````{tab-set-code}
```{code-block} rust
use iceoryx2_bb_container::string::StaticString;

type KeyType = StaticString<50>;
let service = node.service_builder(&"global_config".try_into()?)
    .blackboard_creator::<KeyType>()
    // low battery warning when load is below 25%
    .add::<f32>(StaticString::try_from("battery_threshold")?, 0.25)
    // default ultrasonic update rate = 100 ms
    .add::<u32>(
        StaticString::try_from("ultra_sonic_sensor_update_rate_in_ms")?,
        100,
    )
    .create()?;
```

```{code-block} python
battery_key = ctypes.c_uint64(0)
us_sensor_key = ctypes.c_uint64(1)

service = (
    node.service_builder(iox2.ServiceName.new("My/Funk/ServiceName"))
    .blackboard_creator(ctypes.c_uint64)
    .add(battery_key, ctypes.c_float(0.25))
    .add(us_sensor_key, ctypes.c_uint32(100))
    .create()
)
```

```{code-block} c++
#include "iox2/container/static_string.hpp"

 using KeyType = container::StaticString<50>;
 auto battery_key = container::StaticString<50>::from_utf8("battery_threshold");
 auto us_sensor_key = container::StaticString<50>::from_utf8("ultra_sonic_sensor_update_rate_in_ms");
 if (!battery_key.has_value() || !us_sensor_key.has_value()) {
     std::cerr << "Blackboard keys could not be created." << std::endl;
 }

 auto service = node.service_builder(ServiceName::create("global_config").expect(""))
                    .blackboard_creator<KeyType>()
                    // low battery warning when load is below 25%
                    .template add<float>(battery_key.value(), 0.25)
                    // default ultrasonic update rate = 100 ms
                    .template add<uint32_t>(us_sensor_key.value(), 100)
                    .create()
                    .expect("");
```

```{code-block} c
// create service name
const char* service_name_value = "global_config";
iox2_service_name_h service_name = NULL;
if (iox2_service_name_new(NULL, service_name_value, strlen(service_name_value), &service_name) != IOX2_OK) {
    printf("Unable to create service name!\n");
    goto drop_node;
}

// create service builder
iox2_service_name_ptr service_name_ptr = iox2_cast_service_name_ptr(service_name);
iox2_service_builder_h service_builder = iox2_node_service_builder(&node, NULL, service_name_ptr);
iox2_service_builder_blackboard_creator_h service_builder_blackboard =
    iox2_service_builder_blackboard_creator(service_builder);

// set key type
const char* key_type_name = "uint64_t";
if (iox2_service_builder_blackboard_creator_set_key_type_details(
        &service_builder_blackboard, key_type_name, strlen(key_type_name), sizeof(uint64_t), alignof(uint64_t))
    != IOX2_OK) {
    printf("Unable to set key type details!\n");
    goto drop_service_name;
}

// set key eq comparison function
iox2_service_builder_blackboard_creator_set_key_eq_comparison_function(&service_builder_blackboard, key_cmp);

// create key-value pairs
uint64_t battery_key = 0;
const char* battery_value_type_name = "float";
float battery_value = 0.25;
uint64_t us_sensor_key = 1;
const char* us_sensor_value_type_name = "uint32_t";
uint32_t us_sensor_value = 100;

iox2_service_builder_blackboard_creator_add(&service_builder_blackboard,
                                            &battery_key,
                                            &battery_value,
                                            NULL,
                                            battery_value_type_name,
                                            strlen(battery_value_type_name),
                                            sizeof(float),
                                            alignof(float));

iox2_service_builder_blackboard_creator_add(&service_builder_blackboard,
                                            &us_sensor_key,
                                            &us_sensor_value,
                                            NULL,
                                            us_sensor_value_type_name,
                                            strlen(us_sensor_value_type_name),
                                            sizeof(uint32_t),
                                            alignof(uint32_t));

// create service
iox2_port_factory_blackboard_h service = NULL;
if (iox2_service_builder_blackboard_create(service_builder_blackboard, NULL, &service) != IOX2_OK) {
    printf("Unable to create service!\n");
    goto drop_service_name;
}

// do not forget to release the resources later
drop_service_name:
    iox2_service_name_drop(service_name);
drop_node:
    iox2_node_drop(node);

```
````

Now we create a writer port to update values:

````{tab-set-code}
```{code-block} rust
let writer = service.writer_builder().create()?;
```

```{code-block} python
writer = service.writer_builder().create()
```

```{code-block} c++
auto writer = service.writer_builder().create().expect("");
```

```{code-block} c
iox2_port_factory_writer_builder_h writer_builder = iox2_port_factory_blackboard_writer_builder(&service, NULL);
iox2_writer_h writer = NULL;
if (iox2_port_factory_writer_builder_create(writer_builder, NULL, &writer) != IOX2_OK) {
    printf("Unable to create writer!\n");
    goto drop_service;
}

// do not forget to release the resources later
drop_service:
    iox2_port_factory_blackboard_drop(service);
```
````

To update an entry, we first obtain a handle for it. The handle is type-checked,
so if you provide the wrong type, you’ll get an error instead of silently
corrupting the data.

````{tab-set-code}
```{code-block} rust
let mut battery_threshold_handle =
    writer.entry::<f32>(&"battery_threshold".try_into()?)?;
let mut update_rate_handle =
    writer.entry::<u32>(&"ultra_sonic_sensor_update_rate_in_ms".try_into()?)?;
```

```{code-block} python
battery_threshold_handle = writer.entry(battery_key, ctypes.c_float)
update_rate_handle = writer.entry(us_sensor_key, ctypes.c_uint32)
```

```{code-block} c++
auto battery_threshold_handle = writer.template entry<float>(battery_key.value()).expect("");
auto update_rate_handle = writer.template entry<uint32_t>(us_sensor_key.value()).expect("");
```

```{code-block} c
iox2_entry_handle_mut_h battery_threshold_handle = NULL;
if (iox2_writer_entry(&writer,
                      NULL,
                      &battery_threshold_handle,
                      &battery_key,
                      battery_value_type_name,
                      strlen(battery_value_type_name),
                      sizeof(float),
                      alignof(float))
    != IOX2_OK) {
    printf("Unable to create battery threshold handle!\n");
    goto drop_writer;
}
iox2_entry_handle_mut_h update_rate_handle = NULL;
if (iox2_writer_entry(&writer,
                      NULL,
                      &update_rate_handle,
                      &us_sensor_key,
                      us_sensor_value_type_name,
                      strlen(us_sensor_value_type_name),
                      sizeof(uint32_t),
                      alignof(uint32_t))
    != IOX2_OK) {
    printf("Unable to create update rate handle!\n");
    goto drop_battery_threshold_handle;
}

// do not forget to release the resources later
    iox2_entry_handle_mut_drop(update_rate_handle);
drop_battery_threshold_handle:
    iox2_entry_handle_mut_drop(battery_threshold_handle);
drop_writer:
    iox2_writer_drop(writer);
```
````

In the event loop, we periodically check for user input and update entries as
needed:

````{tab-set-code}
```{code-block} rust
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

```{code-block} python
try:
    while True:
        node.wait(iox2.Duration.from_millis(100))

        new_battery_threshold = get_battery_threshold()
        if new_battery_threshold is not None:
            # small value -> simple copy API
            battery_threshold_handle.update_with_copy(
                ctypes.c_float(new_battery_threshold)
            )

        new_update_rate = get_update_rate()
        if new_update_rate is not None:
            # larger values -> zero-copy loan API
            value_uninit = update_rate_handle.loan_uninit()
            value = value_uninit.write(ctypes.c_uint32(new_update_rate))
            # loan consumes the handle, returned when the update completes
            update_rate_handle = value.update()

except iox2.NodeWaitFailure:
    print("exit")
```

```{code-block} c++
while (node.wait(iox::units::Duration::fromMilliseconds(100)).has_value()) {
    auto new_battery_threshold = get_battery_threshold();
    if (new_battery_threshold.has_value()) {
        // small value -> simple copy API
        battery_threshold_handle.update_with_copy(new_battery_threshold.value());
    }

    auto new_update_rate = get_update_rate();
    if (new_update_rate.has_value()) {
        // larger values -> zero-copy loan API
        auto value_uninit = loan_uninit(std::move(update_rate_handle));
        auto value = write(std::move(value_uninit), new_update_rate.value());
        // loan consumes the handle, returned when the update completes
        update_rate_handle = update(std::move(value));
    }
}
```

```{code-block} c
while (iox2_node_wait(&node, 0, 100000) == IOX2_OK) {
    float new_battery_threshold = get_battery_threshold();
    // small value -> simple copy API
    iox2_entry_handle_mut_update_with_copy(
        &battery_threshold_handle, &new_battery_threshold, sizeof(float), alignof(float));

    uint32_t new_update_rate = get_update_rate();
    // larger values -> zero-copy loan API
    iox2_entry_value_h value_uninit = NULL;
    iox2_entry_handle_mut_loan_uninit(update_rate_handle, NULL, &value_uninit, sizeof(uint32_t), alignof(uint32_t));
    uint32_t* value = NULL;
    iox2_entry_value_mut(&value_uninit, (void**) &value);
    *value = new_update_rate;
    // loan consumes the handle, returned when the update completes
    iox2_entry_value_update(value_uninit, NULL, &update_rate_handle);
}
```
````

## Reader

On the subscriber side, we reuse the node setup from the publish-subscribe
example and skip to opening the blackboard service:

````{tab-set-code}
```{code-block} rust
type KeyType = StaticString<50>;
let service = node.service_builder(&"global_config".try_into()?)
    .blackboard_opener::<KeyType>()
    .open()?;
```

```{code-block} python
service = (
    node.service_builder(iox2.ServiceName.new("My/Funk/ServiceName"))
    .blackboard_opener(ctypes.c_uint64)
    .open()
)
```

```{code-block} c++
using KeyType = container::StaticString<50>;
auto service = node.service_builder(ServiceName::create("global_config").expect(""))
                   .blackboard_opener<KeyType>()
                   .open()
                   .expect("");
```

```{code-block} c
// create service name
const char* service_name_value = "global_config";
iox2_service_name_h service_name = NULL;
if (iox2_service_name_new(NULL, service_name_value, strlen(service_name_value), &service_name) != IOX2_OK) {
    printf("Unable to create service name!\n");
    goto drop_node;
}

// create service builder
iox2_service_name_ptr service_name_ptr = iox2_cast_service_name_ptr(service_name);
iox2_service_builder_h service_builder = iox2_node_service_builder(&node, NULL, service_name_ptr);
iox2_service_builder_blackboard_opener_h service_builder_blackboard =
    iox2_service_builder_blackboard_opener(service_builder);

// set key type
const char* key_type_name = "uint64_t";
if (iox2_service_builder_blackboard_opener_set_key_type_details(
        &service_builder_blackboard, key_type_name, strlen(key_type_name), sizeof(uint64_t), alignof(uint64_t))
    != IOX2_OK) {
    printf("Unable to set key type details!\n");
    goto drop_service_name;
}

// open service
iox2_port_factory_blackboard_h service = NULL;
if (iox2_service_builder_blackboard_open(service_builder_blackboard, NULL, &service) != IOX2_OK) {
    printf("Unable to open service!\n");
    goto drop_service_name;
}

// do not forget to release the resources later
drop_service_name:
    iox2_service_name_drop(service_name);
drop_node:
    iox2_node_drop(node);
```
````

And create a reader port:

````{tab-set-code}
```{code-block} rust
let reader = service.reader_builder().create()?;
```

```{code-block} python
reader = service.reader_builder().create()
```

```{code-block} c++
auto reader = service.reader_builder().create().expect("");
```

```{code-block} c
iox2_port_factory_reader_builder_h reader_builder = iox2_port_factory_blackboard_reader_builder(&service, NULL);
iox2_reader_h reader = NULL;
if (iox2_port_factory_reader_builder_create(reader_builder, NULL, &reader) != IOX2_OK) {
    printf("Unable to create reader!\n");
    goto drop_service;
}

// do not forget to release the resources later
drop_service:
    iox2_port_factory_blackboard_drop(service);
```
````

We’re interested in the `ultra_sonic_sensor_update_rate_in_ms`. We obtain a
handle that always points to the latest value:

````{tab-set-code}
```{code-block} rust
let update_rate_handle =
    reader.entry::<u32>(&"ultra_sonic_sensor_update_rate_in_ms".try_into()?)?;
```

```{code-block} python
update_rate_handle = reader.entry(us_sensor_key, ctypes.c_uint32)
```

```{code-block} c++
auto update_rate_handle = reader.template entry<uint32_t>(us_sensor_key.value()).expect("");
```

```{code-block} c
uint64_t us_sensor_key = 1;
const char* us_sensor_value_type_name = "uint32_t";
iox2_entry_handle_h update_rate_handle = NULL;
if (iox2_reader_entry(&reader,
                      NULL,
                      &update_rate_handle,
                      &us_sensor_key,
                      us_sensor_value_type_name,
                      strlen(us_sensor_value_type_name),
                      sizeof(uint32_t),
                      alignof(uint32_t))
    != IOX2_OK) {
    printf("Unable to create update rate handle!\n");
    goto drop_reader;
}

// do not forget to release the resources later
drop_reader:
    iox2_reader_drop(reader);
```
````

The sensor loop looks almost identical to our original publisher code, except
that the update interval now comes from the global configuration:

````{tab-set-code}
```{code-block} rust
while node.wait(Duration::from_millis(update_rate_handle.get() as u64)).is_ok() {
    let sample = publisher.loan_uninit()?;

    let sample = sample.write_payload(Distance {
        distance_in_meters: get_ultra_sonic_sensor_distance(),
        some_other_property: 42.0,
    });

    sample.send()?;
}
```

```{code-block} python
try:
    while True:
        node.wait(
            iox2.Duration.from_millis(
                update_rate_handle.get().decode_as(ctypes.c_uint32).value
            )
        )

        sample = publisher.loan_uninit()

        d = get_ultrasonic_sensor_distance()
        sample = sample.write_payload(
            Distance(distance_in_meters=d, some_other_property=42.0)
        )

        sample.send()

except iox2.NodeWaitFailure:
    print("exit")
```

```{code-block} c++
while (node.wait(iox::units::Duration::fromMilliseconds(update_rate_handle.get())).has_value()) {
    auto sample = publisher.loan_uninit().expect("");

    auto initialized_sample = sample.write_payload(Distance { get_ultra_sonic_sensor_distance(), 42.0 });

    send(std::move(initialized_sample)).expect("");
}
```

```{code-block} c
uint32_t new_update_rate = 0;
iox2_entry_handle_get(&update_rate_handle, &new_update_rate, sizeof(uint32_t), alignof(uint32_t));
while (iox2_node_wait(&node, 0, new_update_rate * 1000) == IOX2_OK) {
    // loan sample
    iox2_sample_mut_h sample = NULL;
    if (iox2_publisher_loan_slice_uninit(&publisher, NULL, &sample, 1) != IOX2_OK) {
        printf("Failed to loan sample\n");
        goto drop_update_rate_handle;
    }

    // write payload
    struct Distance* payload = NULL;
    iox2_sample_mut_payload_mut(&sample, (void**) &payload, NULL);
    payload->distance_in_meters = get_ultra_sonic_sensor_distance();
    payload->some_other_property = 42.0;

    // send sample
    if (iox2_sample_mut_send(sample, NULL) != IOX2_OK) {
        printf("Failed to send sample\n");
        goto drop_update_rate_handle;
    }
    iox2_entry_handle_get(&update_rate_handle, &new_update_rate, sizeof(uint32_t), alignof(uint32_t));
}

// do not forget to release the resources later
drop_update_rate_handle:
    iox2_entry_handle_drop(update_rate_handle);
```
````

## Related Examples

<!-- markdownlint-disable-next-line MD033 -->
<br/>

````{grid} 1 1 2 2

```{grid-item}
### {octicon}`code` Rust

{octicon}`mark-github` [Minimal Creator/Writer](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/blackboard/creator.rs)
{octicon}`mark-github` [Minimal Opener/Reader](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/blackboard/opener.rs)
```

```{grid-item}
**{octicon}`code` Python**

{octicon}`mark-github` [Minimal Creator/Writer](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/blackboard/creator.rs)
{octicon}`mark-github` [Minimal Opener/Reader](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/blackboard/opener.rs)
```

```{grid-item}
**{octicon}`code` C++**

{octicon}`mark-github` [Minimal Creator/Writer](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/blackboard/src/creator.cpp)
{octicon}`mark-github` [Minimal Opener/Reader](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/blackboard/src/opener.cpp)
```

```{grid-item}
**{octicon}`code` C**

{octicon}`mark-github` [Minimal Creator/Writer](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/blackboard/src/creator.c)
{octicon}`mark-github` [Minimal Opener/Reader](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/blackboard/src/opener.c)
```


````
