# Event-Driven Communication

Before jumping into the example, we need to clarify the difference between
**data flow** and **control flow**.

In programming, control flow is about _when_ a function executes, while data
flow is about _what inputs_ it receives. In iceoryx2 these two concepts are
separated on purpose, unlike in many network protocols where receiving data
automatically involves a syscall that wakes up the process.

Messaging patterns like publish–subscribe and request–response define the data
flow. The event messaging pattern handles control flow. Why split them? Because
you don’t always want to pass data when triggering execution. Sometimes you just
need to wake up a function with no arguments. Other times you need several
arguments, and you don’t want to be interrupted until all of them have arrived.
Mixing data and control flow forces you into hacks like sending empty messages
or waking up too early.

By keeping them separate, iceoryx2 gives you explicit control over _when_ your
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

````{tab-set-code}
```{code-block} rust
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

```{code-block} python
import iceoryx2 as iox2

node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)

pubsub_service = (
    node.service_builder(iox2.ServiceName.new("distance_to_obstacle"))
    .publish_subscribe(Distance)
    .subscriber_max_buffer_size(3)
    .history_size(3)
    .subscriber_max_borrowed_samples(3)
    .open_or_create()
)
```

```{code-block} c++
#include "iox2/iceoryx2.hpp"

using namespace iox2;

auto node = NodeBuilder().create<ServiceType::Ipc>().expect("");

auto pubsub_service = node.service_builder(ServiceName::create("distance_to_obstacle").expect(""))
                   .publish_subscribe<Distance>()
                   .subscriber_max_buffer_size(3)
                   .history_size(3)
                   .subscriber_max_borrowed_samples(3)
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
                                                          sizeof(struct TransmissionData),
                                                          alignof(struct TransmissionData))
    != IOX2_OK) {
    printf("Unable to set type details\n");
    exit(-1);
}

iox2_service_builder_pub_sub_set_subscriber_max_buffer_size(&service_builder_pub_sub, 3);
iox2_service_builder_pub_sub_set_history_size(&service_builder_pub_sub, 3);
iox2_service_builder_pub_sub_set_subscriber_max_borrowed_samples(&service_builder_pub_sub, 3);

// create service
iox2_port_factory_pub_sub_h pubsub_service = NULL;
if (iox2_service_builder_pub_sub_open_or_create(service_builder_pub_sub, NULL, &pubsub_service) != IOX2_OK) {
    printf("Unable to create service!\n");
    exit(-1);
}

// do not forget to release the resources later
iox2_port_factory_pub_sub_drop(pubsub_service);
iox2_service_name_drop(service_name);
iox2_node_drop(node_handle);
```
````

For the event service, we configure a special event that fires if the sensor
process is identified as dead, allowing the emergency brake to switch to a safe
state.

````{tab-set-code}
```{code-block} rust
let ultra_sonic_service_dead = EventId::new(10);
let event_service = node
    .service_builder(&"distance_to_obstacle".try_into()?)
    .event()
    .notifier_dead_event(ultra_sonic_service_dead)
    .open_or_create()?;
```

```{code-block} python
ultra_sonic_service_dead = iox2.EventId.new(10)
event_service = (
    node
    .service_builder(iox2.ServiceName.new("distance_to_obstacle"))
    .event()
    .notifier_dead_event(ultra_sonic_service_dead)
    .open_or_create()
)
```

```{code-block} c++
auto ultra_sonic_service_dead = EventId::new(10);
auto event_service = node.service_builder(ServiceName::create("distance_to_obstacle").expect(""))
                   .event()
                   .notifier_dead_event(ultra_sonic_service_dead)
                   .open_or_create()
                   .expect("");
```

```{code-block} c
size_t ultra_sonic_service_dead = 10;
iox2_service_name_ptr service_name_ptr = iox2_cast_service_name_ptr(service_name);
iox2_service_builder_h service_builder = iox2_node_service_builder(&node_handle, NULL, service_name_ptr);
iox2_service_builder_event_h service_builder_event = iox2_service_builder_event(service_builder);

iox2_service_builder_event_set_notifier_dead_event(&service_builder_event, ultra_sonic_service_dead);

iox2_port_factory_event_h event_service = NULL;
if (iox2_service_builder_event_open_or_create(service_builder_event, NULL, &event_service) != IOX2_OK) {
    printf("Unable to create service!\n");
    exit(-1)
}

// do not forget to release the resources later
iox2_port_factory_event_drop(event_service);
```
````

Now we create a publisher for the distance samples and a notifier for control
events.

````{tab-set-code}
```{code-block} rust
let publisher = pubsub_service.publisher_builder().create()?;
let notifier = event_service.notifier_builder().create()?;

let obstacle_too_close = EventId::new(5);
```

```{code-block} python
publisher = pubsub_service.publisher_builder().create()
notifier = event_service.notifier_builder().create()

obstacle_too_close = iox2.EventId.new(5)
```

```{code-block} c++
auto publisher = pubsub_service.publisher_builder().create().expect("");
auto notifier = event_service.notifier_builder().create().expect("");

auto obstacle_too_close = EventId(5);
```

```{code-block} c
// create notifier
iox2_port_factory_notifier_builder_h notifier_builder = iox2_port_factory_event_notifier_builder(&event_service, NULL);
iox2_notifier_h notifier = NULL;
if (iox2_port_factory_notifier_builder_create(notifier_builder, NULL, &notifier) != IOX2_OK) {
    printf("Unable to create notifier!\n");
    exit(-1);
}

// create publisher
iox2_port_factory_publisher_builder_h publisher_builder =
    iox2_port_factory_pub_sub_publisher_builder(&pubsub_service, NULL);
iox2_publisher_h publisher = NULL;
if (iox2_port_factory_publisher_builder_create(publisher_builder, NULL, &publisher) != IOX2_OK) {
    printf("Unable to create publisher!\n");
    exit(-1);
}

iox2_event_id_t obstacle_too_close = { .value = 5 };

// do not forget to release the resources later
iox2_publisher_drop(publisher);
iox2_notifier_drop(notifier);
```
````

The publishing loop: send the distance sample every 100 ms, and trigger an event
if it’s below the threshold.

````{tab-set-code}
```{code-block} rust
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

```{code-block} python
try:
    while True:
        node.wait(iox2.Duration.from_millis(100))

        sample = publisher.loan_uninit()

        distance = get_ultra_sonic_sensor_distance();
        sample = sample.write_payload(
            Distance(distance_in_meters: distance, some_other_property: 42.0)
        )
        sample.send()

        if distance < distance_threshold:
            notifier.notify_with_custom_event_id(obstacle_too_close)

except iox2.NodeWaitFailure:
    print("exit")
```

```{code-block} c++
while (node.wait(iox::units:Duration::fromMilliseconds(100)).has_value()) {
    auto sample = publisher.loan_uninit().expect("acquire sample");

    auto distance = get_ultra_sonic_sensor_distance();
    auto initialized_sample =
        sample.write_payload(Distance { distance, 42.0 });

    if (distance < distance_threshold) {
        notifier.notify_with_custom_event_id(obstacle_too_close).expect("");
    }
}
```

```{code-block} c
while (iox2_node_wait(&node_handle, 0, 100) == IOX2_OK) {
    double distance = get_ultra_sonic_sensor_distance();

    // loan sample
    iox2_sample_mut_h sample = NULL;
    if (iox2_publisher_loan_slice_uninit(&publisher, NULL, &sample, 1) != IOX2_OK) {
        printf("Failed to loan sample\n");
        exit(-1);
    }

    // write payload
    struct Distance* payload = NULL;
    iox2_sample_mut_payload_mut(&sample, (void**) &payload, NULL);
    payload->distance_in_meters = distance;
    payload->some_other_property = 42.0;

    // send sample
    if (iox2_sample_mut_send(sample, NULL) != IOX2_OK) {
        printf("Failed to send sample\n");
        exit(-1);
    }

    if (distance < distance_threshold) {
        if (iox2_notifier_notify_with_custom_event_id(&notifier, &obstacle_too_close, NULL) != IOX2_OK) {
            printf("Failed to notify listener!\n");
            exit(-1);
        }
    }
}
```
````

## Subscriber Setup

On the other side, we again create both services and their ports:

````{tab-set-code}
```{code-block} rust
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

let ultra_sonic_service_dead = EventId::new(10);
let event_service = node
    .service_builder(&"distance_to_obstacle".try_into()?)
    .event()
    .notifier_dead_event(ultra_sonic_service_dead)
    .open_or_create()?;

let subscriber = pubsub_service.subscriber_builder().create()?;
let listener = event_service.listener_builder().create()?;
```

```{code-block} python
import iceoryx2 as iox2

node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)

pubsub_service = (
    node.service_builder(iox2.ServiceName.new("distance_to_obstacle"))
    .publish_subscribe(Distance)
    .subscriber_max_buffer_size(3)
    .history_size(3)
    .subscriber_max_borrowed_samples(3)
    .open_or_create()
)

ultra_sonic_service_dead = iox2.EventId.new(10)
event_service = (
    node
    .service_builder(iox2.ServiceName.new("distance_to_obstacle"))
    .event()
    .notifier_dead_event(ultra_sonic_service_dead)
    .open_or_create()
)

subscriber = pubsub_service.subscriber_builder().create()
listener = event_service.listener_builder().create()
```

```{code-block} c++
#include "iox2/iceoryx2.hpp"

using namespace iox2;

auto node = NodeBuilder().create<ServiceType::Ipc>().expect("");

auto pubsub_service = node.service_builder(ServiceName::create("distance_to_obstacle").expect(""))
                   .publish_subscribe<Distance>()
                   .subscriber_max_buffer_size(3)
                   .history_size(3)
                   .subscriber_max_borrowed_samples(3)
                   .open_or_create()
                   .expect("");

auto ultra_sonic_service_dead = EventId::new(10);
auto event_service = node.service_builder(ServiceName::create("distance_to_obstacle").expect(""))
                   .event()
                   .notifier_dead_event(ultra_sonic_service_dead)
                   .open_or_create()
                   .expect("");

auto subscriber = pubsub_service.subscriber_builder().create().expect("");
auto listener = event_service.listener_builder().create().expect("");
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
                                                          sizeof(struct TransmissionData),
                                                          alignof(struct TransmissionData))
    != IOX2_OK) {
    printf("Unable to set type details\n");
    exit(-1);
}

iox2_service_builder_pub_sub_set_subscriber_max_buffer_size(&service_builder_pub_sub, 3);
iox2_service_builder_pub_sub_set_history_size(&service_builder_pub_sub, 3);
iox2_service_builder_pub_sub_set_subscriber_max_borrowed_samples(&service_builder_pub_sub, 3);

// create pubsub service
iox2_port_factory_pub_sub_h pubsub_service = NULL;
if (iox2_service_builder_pub_sub_open_or_create(service_builder_pub_sub, NULL, &pubsub_service) != IOX2_OK) {
    printf("Unable to create service!\n");
    exit(-1);
}

// create event service
size_t ultra_sonic_service_dead = 10;
iox2_service_name_ptr service_name_ptr = iox2_cast_service_name_ptr(service_name);
iox2_service_builder_h service_builder = iox2_node_service_builder(&node_handle, NULL, service_name_ptr);
iox2_service_builder_event_h service_builder_event = iox2_service_builder_event(service_builder);

iox2_service_builder_event_set_notifier_dead_event(&service_builder_event, ultra_sonic_service_dead);

iox2_port_factory_event_h event_service = NULL;
if (iox2_service_builder_event_open_or_create(service_builder_event, NULL, &event_service) != IOX2_OK) {
    printf("Unable to create service!\n");
    exit(-1);
}

// create subscriber
iox2_port_factory_subscriber_builder_h subscriber_builder =
    iox2_port_factory_pub_sub_subscriber_builder(&pubsub_service, NULL);
iox2_subscriber_h subscriber = NULL;
if (iox2_port_factory_subscriber_builder_create(subscriber_builder, NULL, &subscriber) != IOX2_OK) {
    printf("Unable to create subscriber!\n");
    exit(-1);
}

// create listener
iox2_port_factory_listener_builder_h listener_builder = iox2_port_factory_event_listener_builder(&event_service, NULL);
iox2_listener_h listener = NULL;
if (iox2_port_factory_listener_builder_create(listener_builder, NULL, &listener) != IOX2_OK) {
    printf("Unable to create listener!\n");
    exit(-1);
}

// do not forget to release the resources later
iox2_listener_drop(listener);
iox2_subscriber_drop(subscriber);
iox2_port_factory_event_drop(event_service);
iox2_port_factory_pub_sub_drop(pubsub_service);
iox2_service_name_drop(service_name);
iox2_node_drop(node_handle);
```
````

Instead of polling every 100 ms, the subscriber just waits for events. When
woken up, it processes them and goes back to sleep.

````{tab-set-code}
```{code-block} rust
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

```{code-block} python
try:
    while True:
        event_id = listener.blocking_wait_one()
        if event_id == ultra_sonic_service_dead:
            go_into_parking_position()

        if event_id == obstacle_too_close:
            last_samples = []
            while True:
                sample = subscriber.receive()
                if sample is not None:
                    data = sample.payload()
                    last_samples.append(data)
                    if len(last_samples) == 3:
                        break
                else:
                    break

            perform_break(last_samples)

except iox2.ListenerWaitError:
    print("exit")
```

```{code-block} c++
while (listener.blocking_wait_all(
    [](auto event_id) {
        if ( event_id == ultra_sonic_service_dead ) {
            go_into_parking_position();
        }

        if ( event_id == obstacle_too_close ) {
            std::vector<Sample<ServiceType::Ipc, Distance, void>> last_samples;

            auto sample = subscriber.receive().expect("");
            while (sample.has_value()) {
                sample = subscriber.receive().expect("");
                last_samples.push_back(std::move(sample.value()));
                if (last_samples.size() == 3) {
                    break;
                }
            }

            perform_break(last_samples);
        }
    })) {}
```

```{code-block} c
iox2_event_id_t event_id;
while (iox2_node_wait(&node_handle, 0, 0) == IOX2_OK) {
    bool has_received_one = false;
    if (iox2_listener_blocking_wait_one(&listener, &event_id, &has_received_one) != IOX2_OK) {
        printf("Unable to wait for notification!\n");
        exit(-1);
    }

    if (has_received_one && event_id.value == ultra_sonic_service_dead.value) {
        go_into_parking_position();
    }

    if (has_received_one && event_id.value == obstacle_too_close.value) {
        iox2_sample_h last_samples[3];
        for(uint64_t i = 0; i < 3; ++i) {
            if (iox2_subscriber_receive(&subscriber, NULL, &last_samples[i]) != IOX2_OK) {
                printf("Failed to receive sample\n");
                exit(-1);
            }

            if ( last_samples[i] == NULL ) {
                break;
            }
        }

        perform_break(last_samples);

        for(uint64_t i = 0; i < 3; ++i) {
            if (last_samples[i] != NULL) {
                iox2_sample_drop(last_samples[i]);
            }
        }
    }
}
```
````

## Health Monitoring

The notifier’s “dead event” relies on health monitoring: iceoryx2 offers
building blocks to detects when a process dies. How this works in detail is
covered in a separate tutorial.

## Source Code

````{tab-set}
```{tab-item} RUST
* [GitHub Rust Publisher Event-Based-Communication-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/event_based_communication/publisher.rs)
* [GitHub Rust Subscriber Event-Based-Communication-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/event_based_communication/subscriber.rs)
```

```{tab-item} PYTHON
The functionality is fully available in Python, but no example has been written
yet. Feel free to create a pull request and contribute one.
```

```{tab-item} C++
* [GitHub C++ Publisher Event-Based-Communication-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/event_based_communication/src/publisher.cpp)
* [GitHub C++ Subscriber Event-Based-Communication-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/event_based_communication/src/subscriber.cpp)
```

```{tab-item} C
The functionality is fully available in C, but no example has been written
yet. Feel free to create a pull request and contribute one.
```
````
