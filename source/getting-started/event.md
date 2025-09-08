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

````{tab-set-code}
```{code-block} rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new().create::<ipc::Service>()?;

let event_service = node
    .service_builder(&"system health events".try_into()?)
    .event()
    .open_or_create()?;
```

```{code-block} python
import iceoryx2 as iox2

iox2.set_log_level_from_env_or(iox2.LogLevel.Info)
node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)

event = (
    node
    .service_builder(iox2.ServiceName.new("system health events"))
    .event()
    .open_or_create()
)
```

```{code-block} c++
#include "iox2/iceoryx2.hpp"

using namespace iox2;

auto node = NodeBuilder().create<ServiceType::Ipc>().expect("");

auto service = node.service_builder(ServiceName::create("system health events").expect(""))
                   .event()
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
const char* service_name_value = "system health events";
iox2_service_name_h service_name = NULL;
if (iox2_service_name_new(NULL, service_name_value, strlen(service_name_value), &service_name) != IOX2_OK) {
    printf("Unable to create service name!\n");
    exit(-1);
}

// create service
iox2_service_name_ptr service_name_ptr = iox2_cast_service_name_ptr(service_name);
iox2_service_builder_h service_builder = iox2_node_service_builder(&node_handle, NULL, service_name_ptr);
iox2_service_builder_event_h service_builder_event = iox2_service_builder_event(service_builder);
iox2_port_factory_event_h service = NULL;
if (iox2_service_builder_event_open_or_create(service_builder_event, NULL, &service) != IOX2_OK) {
    printf("Unable to create service!\n");
    exit(-1);
}

// do not forget to release the resources later
iox2_service_name_drop(service_name);
iox2_port_factory_pub_sub_drop(service);
iox2_node_drop(node);
```
````

Now we create the notifier port, which sends notifications:

````{tab-set-code}
```{code-block} rust
let notifier = event_service.notifier_builder().create()?;
```

```{code-block} python
notifier = event.notifier_builder().create()
```

```{code-block} c++
auto notifier = service.notifier_builder().create().expect("");
```

```{code-block} c
iox2_port_factory_notifier_builder_h notifier_builder = iox2_port_factory_event_notifier_builder(&service, NULL);
iox2_notifier_h notifier = NULL;
if (iox2_port_factory_notifier_builder_create(notifier_builder, NULL, &notifier) != IOX2_OK) {
    printf("Unable to create notifier!\n");
    exit(-1);
}

// do not forget to release the resources later
iox2_notifier_drop(notifier);
```
````

We want to notify about two distinct things: wall collisions and low battery.
So we define two event IDs:

````{tab-set-code}
```{code-block} rust
let wall_was_hit = EventId::new(0);
let battery_is_low = EventId::new(1);
```

```{code-block} python
wall_was_hit = iox2.EventId.new(0)
battery_is_low = iox2.EventId.new(1)
```

```{code-block} c++
const auto wall_was_hit = EventId(0);
const auto battery_is_low = EventId(1);
```

```{code-block} c
iox2_event_id_t wall_was_hit = { .value = 0 };
iox2_event_id_t battery_is_low = { .value = 1 };
```
````

Checking the sensors every second is good enough for health monitoring, so we
send notifications only when something changes:

````{tab-set-code}
```{code-block} rust
while node.wait(Duration::from_secs(1)).is_ok() {
    if bump_sensor_was_activated() {
        notifier.notify_with_custom_event_id(wall_was_hit)?;
    }

    if battery_state() < battery_threshold {
        notifier.notify_with_custom_event_id(battery_is_low)?;
    }
}
```

```{code-block} python
try:
    while True:
        node.wait(iox2.Duration.from_secs(1))
        if bump_sensor_was_activated():
            notifier.notify_with_custom_event_id(wall_was_hit)

        if battery_state() < battery_threshold:
            notifier.notify_with_custom_event_id(battery_is_low)

except iox2.NodeWaitFailure:
    print("exit")
```

```{code-block} c++
while (node.wait(iox::units::Duration::fromSeconds(1)).has_value()) {
    if ( bump_sensor_was_activated() ) {
        notifier.notify_with_custom_event_id(wall_was_hit).expect("");
    }

    if ( battery_state() < battery_threshold ) {
        notifier.notify_with_custom_event_id(battery_is_low).expect("");
    }
}
```

```{code-block} c
while (iox2_node_wait(&node_handle, 1, 0) == IOX2_OK) {
    if ( bump_sensor_was_activated() ) {
        if (iox2_notifier_notify_with_custom_event_id(&notifier, &wall_was_hit, NULL) != IOX2_OK) {
            printf("Failed to notify listener!\n");
            exit(-1);
        }
    }

    if ( battery_state() < battery_threshold ) {
        if (iox2_notifier_notify_with_custom_event_id(&notifier, &battery_is_low, NULL) != IOX2_OK) {
            printf("Failed to notify listener!\n");
            exit(-1);
        }
    }
}
```
````

## Listener

On the other side, the listener is the process waiting for the health events.
First,  create the node and open the event service:

````{tab-set-code}
```{code-block} rust
use iceoryx2::prelude::*;

let node = NodeBuilder::new().create::<ipc::Service>()?;

let event_service = node
    .service_builder(&"system health events".try_into()?)
    .event()
    .open_or_create()?;
```

```{code-block} python
import iceoryx2 as iox2

node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)

event = (
    node
    .service_builder(iox2.ServiceName.new("system health events"))
    .event()
    .open_or_create()
)
```

```{code-block} c++
#include "iox2/iceoryx2.hpp"

using namespace iox2;

auto node = NodeBuilder().create<ServiceType::Ipc>().expect("");

auto service = node.service_builder(ServiceName::create("system health events").expect(""))
                   .event()
                   .open_or_create()
                   .expect("");
```

```{code-block} c
#include "iox2/iceoryx2.h"

iox2_node_builder_h node_builder_handle = iox2_node_builder_new(NULL);
iox2_node_h node_handle = NULL;
if (iox2_node_builder_create(node_builder_handle, NULL, iox2_service_type_e_IPC, &node_handle) != IOX2_OK) {
    printf("Could not create node!\n");
    exit(-1);
}

// create service name
const char* service_name_value = "system health events";
iox2_service_name_h service_name = NULL;
if (iox2_service_name_new(NULL, service_name_value, strlen(service_name_value), &service_name) != IOX2_OK) {
    printf("Unable to create service name!\n");
    exit(-1);
}

// create service
iox2_service_name_ptr service_name_ptr = iox2_cast_service_name_ptr(service_name);
iox2_service_builder_h service_builder = iox2_node_service_builder(&node_handle, NULL, service_name_ptr);
iox2_service_builder_event_h service_builder_event = iox2_service_builder_event(service_builder);
iox2_port_factory_event_h service = NULL;
if (iox2_service_builder_event_open_or_create(service_builder_event, NULL, &service) != IOX2_OK) {
    printf("Unable to create service!\n");
    exit(-1);
}

// do not forget to release the resources later
iox2_port_factory_event_drop(service);
iox2_service_name_drop(service_name);
iox2_node_drop(node_handle);
```
````

Now we create the listener port:

````{tab-set-code}
```{code-block} rust
let listener = event_service.listener_builder().create()?;
```

```{code-block} python
listener = event.listener_builder().create()
```

```{code-block} c++
auto listener = service.listener_builder().create().expect("");
```

```{code-block} c
iox2_port_factory_listener_builder_h listener_builder = iox2_port_factory_event_listener_builder(&service, NULL);
iox2_listener_h listener = NULL;
if (iox2_port_factory_listener_builder_create(listener_builder, NULL, &listener) != IOX2_OK) {
    printf("Unable to create listener!\n");
    exit(-1);
}

// do not forget to release the resources later
iox2_listener_drop(listener);
```
````

This process doesn’t need to poll. It just blocks until an event arrives, then
checks which one it was:

````{tab-set-code}
```{code-block} rust
while listener.blocking_wait_all(|event_id| {
    if event_id == battery_is_low {
        activate_battery_warning_light();
    }
    if event_id == wall_was_hit {
        go_into_parking_position();
    }
}).is_ok() {}
```

```{code-block} python
try:
    while True:
        event_id = listener.blocking_wait_one()
        if event_id == battery_is_low:
            activate_battery_warning_light()
        if event_id == wall_was_hit:
            go_into_parking_position()

except iox2.ListenerWaitError:
    print("exit")
```

```{code-block} c++
while (listener.blocking_wait_all(
    [](auto event_id) {
        if ( event_id == battery_is_low ) {
            activate_battery_warning_light();
        }

        if ( event_id == wall_was_hit ) {
            go_into_parking_position();
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

    if (has_received_one) {
        if ( event_id.value == battery_is_low.value ) {
            activate_battery_warning_light();
        }

        if ( event_id.value == wall_was_hit.value ) {
            go_into_parking_position();
        }
    }
}
```
````

## Source Code

````{tab-set}
```{tab-item} RUST
* [GitHub Rust Listener-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/event/listener.rs)
* [GitHub Rust Notifier-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/event/notifier.rs)
```

```{tab-item} PYTHON
* [GitHub Python Listener-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/event/listener.py)
* [GitHub Python Notifier-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/event/notifier.py)
```

```{tab-item} C++
* [GitHub C++ Listener-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/event/src/listener.cpp)
* [GitHub C++ Notifier-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/event/src/notifier.cpp)
```

```{tab-item} C
* [GitHub C Listener-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/event/src/listener.c)
* [GitHub C Notifier-Example](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/event/src/notifier.c)
```
````
