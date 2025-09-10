# Publish–Subscribe

```{admonition} Learning Objectives

TODO
```

Larry’s hardware and algorithms are already built. Now we want to send the
distance measured by his ultrasonic sensor to another process that can slam the
brakes if an obstacle gets closer than X meters.

In `iceoryx2`, that’s a perfect job for the **publish–subscribe** pattern: one
participant publishes a stream of distances, another subscribes and reacts.

```{figure} /images/wiring-a-robot-brain-publish-subscribe-graph.svg
:alt: Communication graph of Ultrasonic Sensor and Emergency Brake
:align: center
:name: fig-wiring-a-robot-brain-publish-subscribe

The communication graph of the Ultrasonic Sensor and Emergency Brake
participants
```

## Publisher

Everything in `iceoryx2` starts with a node. A node represents a communication
point (like a process or thread) and acts as a factory for services. To keep
things sane when debugging later, we’ll give this node a name:

````{tab-set-code}
```{code-block} rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new()
    .name(&"UltraSonicSensor".try_into()?)
    .create::<ipc::Service>()?;
```

```{code-block} python
import iceoryx2 as iox2

node = (
    iox2.NodeBuilder.new()
    .name(iox2.NodeName.new("UltraSonicSensor"))
    .create(iox2.ServiceType.Ipc)
)
```

```{code-block} c++
#include "iceoryx2.hpp"

using namespace iox2;

auto node = NodeBuilder()
    .name(NodeName::create("UltraSonicSensor").expect("")
    .create<ServiceType::Ipc>().expect("");
```

```{code-block} c
#include "iox2/iceoryx2.h"

iox2_node_builder_h node_builder = iox2_node_builder_new(NULL);

const char* node_name_value = "UltraSonicSensor";
iox2_node_name_h node_name = NULL;
if (iox2_node_name_new(NULL, node_name_value, strlen(node_name_value), &node_name) != IOX2_OK) {
    printf("Unable to set node name!\n");
}

iox2_node_name_ptr node_name_ptr = iox2_cast_node_name_ptr(node_name);
iox2_node_builder_set_name(&node_builder, node_name_ptr);

iox2_node_h node = NULL;
if (iox2_node_builder_create(node_builder, NULL, iox2_service_type_e_IPC, &node) != IOX2_OK) {
    printf("Could not create node!\n");
    exit(-1);
}

// do not forget to release the resources later
iox2_node_name_drop(node_name);
iox2_node_drop(node);
```
````

Now we can create a service called `"distance_to_obstacle"`, using a struct
`Distance` as the payload type. Payloads must be shared-memory compatible,
meaning:

* self-contained
* no internal pointers
* identical memory representation in every process

````{tab-set-code}
```{code-block} rust
#[derive(Debug, ZeroCopySend)]  // every payload must implement ZeroCopySend
#[repr(C)]                      // ensures identical layout across processes
pub struct Distance {
    pub distance_in_meters: f64,
    pub some_other_property: f32,
}
```

```{code-block} python
import ctypes

class TransmissionData(ctypes.Structure):
    _fields_ = [
        ("distance_in_meters", ctypes.c_double),
        ("some_other_property", ctypes.c_float),
    ]

    def __str__(self) -> str:
    return f"Distance {{ distance_in_meters: {self.distance_in_meters}, some_other_property: {self.some_other_property} }}"


```

```{code-block} c++
struct Distance {
    double distance_in_meters;
    float some_other_property;
};

inline auto operator<<(std::ostream& stream, const Distance& value) -> std::ostream& {
    stream << "Distance { distance_in_meters: " << value.distance_in_meters
    stream << ", some_other_property: " << value.some_other_property << " }";
    return stream;
}
```

```{code-block} c
struct Distance {
    double distance_in_meters;
    float some_other_property;
};
```
````

With the payload defined, we can set up the service:

````{tab-set-code}
```{code-block} rust
let service = node
    .service_builder(&"distance_to_obstacle".try_into()?)
    .publish_subscribe::<Distance>()
    .open_or_create()?;
```

```{code-block} python
service = (
    node.service_builder(iox2.ServiceName.new("distance_to_obstacle"))
    .publish_subscribe(Distance)
    .open_or_create()
)
```

```{code-block} c++
auto service = node.service_builder(ServiceName::create("distance_to_obstacle").expect(""))
                   .publish_subscribe<Distance>()
                   .open_or_create()
                   .expect("");
```

```{code-block} c
const char* service_name_value = "distance_to_obstacle";
iox2_service_name_h service_name = NULL;
if (iox2_service_name_new(NULL, service_name_value, strlen(service_name_value), &service_name) != IOX2_OK) {
    printf("Unable to create service name!\n");
    exit(-1);
}

// create service builder
iox2_service_name_ptr service_name_ptr = iox2_cast_service_name_ptr(service_name);
iox2_service_builder_h service_builder = iox2_node_service_builder(&node_handle, NULL, service_name_ptr);
iox2_service_builder_pub_sub_h service_builder_pub_sub = iox2_service_builder_pub_sub(service_builder);

// set pub sub payload type
const char* payload_type_name = "Distance";
if (iox2_service_builder_pub_sub_set_payload_type_details(&service_builder_pub_sub,
                                                          iox2_type_variant_e_FIXED_SIZE,
                                                          payload_type_name,
                                                          strlen(payload_type_name),
                                                          sizeof(struct Distance),
                                                          alignof(struct Distance))
    != IOX2_OK) {
    printf("Unable to set type details\n");
    exit(-1);
}

// create service
iox2_port_factory_pub_sub_h service = NULL;
if (iox2_service_builder_pub_sub_open_or_create(service_builder_pub_sub, NULL, &service) != IOX2_OK) {
    printf("Unable to create service!\n");
    exit(-1);
}

// do not forget to release the resources later
iox2_service_name_drop(service_name);
iox2_port_factory_pub_sub_drop(service);
```
````

Now we create our publisher:

````{tab-set-code}
```{code-block} rust
let publisher = service.publisher_builder().create()?;
```

```{code-block} python
publisher = service.publisher_builder().create()
```

```{code-block} c++
auto publisher = service.publisher_builder().create().expect("");
```

```{code-block} c
iox2_port_factory_publisher_builder_h publisher_builder =
    iox2_port_factory_pub_sub_publisher_builder(&service, NULL);
iox2_publisher_h publisher = NULL;
if (iox2_port_factory_publisher_builder_create(publisher_builder, NULL, &publisher) != IOX2_OK) {
    printf("Unable to create publisher!\n");
    goto drop_service;
}

// do not forget to release the resources later
iox2_publisher_drop(publisher);
```
````

Larry isn’t exactly Formula 1 material, so publishing every 100 ms is plenty. We
set up a loop, wait 100 ms, read the sensor, and send the data:

````{tab-set-code}
```{code-block} rust
while node.wait(Duration::from_millis(100)).is_ok() {
    // acquire sensor reading and send it
}
```

```{code-block} python
try:
    while True:
        node.wait(iox2.Duration.from_millis(100))
        # acquire sensor reading and send it

except iox2.NodeWaitFailure:
    print("exit")
```

```{code-block} c++
while (node.wait(iox::units::Duration::fromMilliseconds(100)).has_value()) {
    // acquire sensor reading and send it
}
```

```{code-block} c
while (iox2_node_wait(&node_handle, 0, 10000000) == IOX2_OK) {
    // acquire sensor reading and send it
}
```
````

To fully benefit from zero-copy, we don’t allocate or clone data. Instead, we
loan an uninitialized sample from the publisher’s memory pool, fill it, and then
send it:

````{tab-set-code}
```{code-block} rust
let sample = publisher.loan_uninit()?;

let sample = sample.write_payload(Distance {
    distance_in_meters: get_ultra_sonic_sensor_distance(),
    some_other_property: 42.0,
});

sample.send()?;
```

```{code-block} python
sample = publisher.loan_uninit()

d = get_ultra_sonic_sensor_distance()
sample = sample.write_payload(
    Distance(distance_in_meters=d, some_other_property=42.0)
)

sample.send()
```

```{code-block} c++
auto sample = publisher.loan_uninit().expect("");

auto initialized_sample =
  sample.write_payload(Distance { get_ultra_sonic_sensor_distance(), 42.0 });

send(std::move(initialized_sample)).expect("");
```

```{code-block} c
// loan sample
iox2_sample_mut_h sample = NULL;
if (iox2_publisher_loan_slice_uninit(&publisher, NULL, &sample, 1) != IOX2_OK) {
    printf("Failed to loan sample\n");
    exit(-1);
}

// write payload
struct Distance* payload = NULL;
iox2_sample_mut_payload_mut(&sample, (void**) &payload, NULL);
payload->distance_in_meters = get_ultra_sonic_sensor_distance();
payload->some_other_property = 42.0;

// send sample
if (iox2_sample_mut_send(sample, NULL) != IOX2_OK) {
    printf("Failed to send sample\n");
    exit(-1);
}
```
````

Whenever we acquire an uninitialized sample, we must write the payload to it and
convert it into an initialized sample. This ensures we don’t accidentally ship
uninitialized garbage across participants. Once the payload is written, the
sample is safe to send.

## Subscriber

The subscriber setup starts the same: create a node, open the
`"distance_to_obstacle"` service, and specify the payload type.

````{tab-set-code}
```{code-block} rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new().create::<ipc::Service>()?;

let service = node
    .service_builder(&"distance_to_obstacle".try_into()?)
    .publish_subscribe::<Distance>()
    .open_or_create()?;
```

```{code-block} python
import iceoryx2 as iox2

node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)

service = (
    node.service_builder(iox2.ServiceName.new("distance_to_obstacle"))
    .publish_subscribe(Distance)
    .open_or_create()
)
```

```{code-block} c++
#include "iox2/iceoryx2.hpp"

using namespace iox2;

auto node = NodeBuilder().create<ServiceType::Ipc>().expect("");

auto service = node.service_builder(ServiceName::create("distance_to_obstacle").expect(""))
                   .publish_subscribe<Distance>()
                   .open_or_create()
                   .expect("");
```

```{code-block} c
#include "iox2/iceoryx2.h"

// create new node
iox2_node_builder_h node_builder_handle = iox2_node_builder_new(NULL);
iox2_node_h node_handle = NULL;
if (iox2_node_builder_create(node_builder_handle, NULL, iox2_service_type_e_IPC, &node_handle) != IOX2_OK) {
    printf("Could not create node!\n");
    exit(-1);
}

// create service name
const char* service_name_value = "distance_to_obstacle";
iox2_service_name_h service_name = NULL;
if (iox2_service_name_new(NULL, service_name_value, strlen(service_name_value), &service_name) != IOX2_OK) {
    printf("Unable to create service name!\n");
    exit(-1);
}

// create service builder
iox2_service_name_ptr service_name_ptr = iox2_cast_service_name_ptr(service_name);
iox2_service_builder_h service_builder = iox2_node_service_builder(&node_handle, NULL, service_name_ptr);
iox2_service_builder_pub_sub_h service_builder_pub_sub = iox2_service_builder_pub_sub(service_builder);

// set pub sub payload type
const char* payload_type_name = "Distance";
if (iox2_service_builder_pub_sub_set_payload_type_details(&service_builder_pub_sub,
                                                          iox2_type_variant_e_FIXED_SIZE,
                                                          payload_type_name,
                                                          strlen(payload_type_name),
                                                          sizeof(struct Distance),
                                                          alignof(struct Distance))
    != IOX2_OK) {
    printf("Unable to set type details\n");
    exit(-1);
}

// create service
iox2_port_factory_pub_sub_h service = NULL;
if (iox2_service_builder_pub_sub_open_or_create(service_builder_pub_sub, NULL, &service) != IOX2_OK) {
    printf("Unable to create service!\n");
    exit(-1);
}

// do not forget to release the resources later
iox2_port_factory_pub_sub_drop(service);
iox2_service_name_drop(service_name);
iox2_node_drop(node_handle);
```
````

Now we create the subscriber side:

````{tab-set-code}
```{code-block} rust
let subscriber = service.subscriber_builder().create()?;
```

```{code-block} python
subscriber = service.subscriber_builder().create()
```

```{code-block} c++
auto subscriber = service.subscriber_builder().create().expect("");
```

```{code-block} c
iox2_port_factory_subscriber_builder_h subscriber_builder =
    iox2_port_factory_pub_sub_subscriber_builder(&service, NULL);
iox2_subscriber_h subscriber = NULL;
if (iox2_port_factory_subscriber_builder_create(subscriber_builder, NULL, &subscriber) != IOX2_OK) {
    printf("Unable to create subscriber!\n");
    exit(-1);
}

// do not forget to release the resources later
iox2_subscriber_drop(subscriber);
```
````

Since the publisher sends updates every 100 ms, we loop at the same pace and
check for new data. If we receive something, we print it:

````{tab-set-code}
```{code-block} rust
while node.wait(Duration::from_millis(100)).is_ok() {
    while let Some(sample) = subscriber.receive()? {
        println!("received distance {:?}", sample.payload());
    }
}
```

```{code-block} python
try:
    while True:
        node.wait(iox2.Duration.from_millis(100))
        while True:
            sample = subscriber.receive()
            if sample is not None:
                data = sample.payload()
                print("received distance:", data.contents)
            else:
                break

except iox2.NodeWaitFailure:
    print("exit")
```

```{code-block} c++
while (node.wait(iox::units::Duration::fromMilliseconds(100)).has_value()) {
    auto sample = subscriber.receive().expect("");
    while (sample.has_value()) {
        std::cout << "received distance: " << sample->payload() << std::endl;
        sample = subscriber.receive().expect("");
    }
}
```

```{code-block} c
while (iox2_node_wait(&node_handle, 0, 10000000) == IOX2_OK) {
    // receive sample
    iox2_sample_h sample = NULL;
    if (iox2_subscriber_receive(&subscriber, NULL, &sample) != IOX2_OK) {
        printf("Failed to receive sample\n");
        exit(-1);
    }

    if (sample != NULL) {
        struct Distance* payload = NULL;
        iox2_sample_payload(&sample, (const void**) &payload, NULL);

        printf("received: Distance { .distance_in_meters: %f, .some_other_property: %f }\n",
               payload->distance_in_meters,
               payload->some_other_property);
        iox2_sample_drop(sample);
    }
}
```
````

From here, reacting is easy: if the distance falls below a threshold, Larry can
hit the emergency brake before smashing into a wall.

## Related Examples

<!-- markdownlint-disable-next-line MD033 -->
<br/>

````{grid} 1 1 2 2
:gutter: 3

```{grid-item}
**{octicon}`code` Rust**

{octicon}`mark-github` [Minimal Publisher](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/publish_subscribe/publisher.rs)  
{octicon}`mark-github` [Minimal Subscriber](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/publish_subscribe/subscriber.rs)
```

```{grid-item}
**{octicon}`code` Python**

{octicon}`mark-github` [Minimal Publisher](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/publish_subscribe/publisher.py)  
{octicon}`mark-github` [Minimal Subscriber](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/publish_subscribe/subscriber.py)
```

```{grid-item}
**{octicon}`code` C++**

{octicon}`mark-github` [Minimal Publisher](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/publish_subscribe/src/publisher.cpp)  
{octicon}`mark-github` [Minimal Subscriber](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/publish_subscribe/src/subscriber.cpp)
```

```{grid-item}
**{octicon}`code` C**

{octicon}`mark-github` [Minimal Publisher](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/publish_subscribe/src/publisher.c)  
{octicon}`mark-github` [Minimal Subscriber](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/publish_subscribe/src/subscriber.c)
```

````
