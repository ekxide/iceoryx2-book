# Event-Driven Gateway

```{important}
The ROS 2 integrations are currently prototypes and have not yet been validated
in real ROS 2 workflows. They are recommended only for experimentation in
development deployments.
```

The previous articles run the gateway in its default polling mode. Every 100
milliseconds the gateway wakes, discovers what changed and moves the pending
samples. The applications behind it must poll their subscribers periodically
as well.

Polling is simple, but it has a cost. A sample is only moved at the polling
frequency. The gateway and each application can delay a sample by up to one
polling period. Along a pipeline, these delays add up. Every poll also wakes
a process when there is nothing to do.

In this article, we will make every component of a pipeline event-driven
instead. The gateway will wake when data arrives from ROS 2 or when an
application publishes. The applications will wake when the gateway delivers
data to them. Samples then move through the pipeline as soon as they are
published.

## Topology

To demonstrate the approach, let's revisit the pipeline from
[Plain Struct as Payload](/tutorials/gateway-to-ros-2/plain-struct-as-payload).
A limiter application in `iceoryx2` subscribes to the `/cmd_vel` topic, clamps
the received velocity to a maximum, and republishes it on `/cmd_vel_limited`.

The data takes the same path as before. What changes is how each component
learns that there is data to handle:

```{mermaid}
:caption: Every component of the pipeline wakes the next one
:alt: ROS 2 publishes Twist -> the gateway wakes on the data -> the limiter wakes on a notification -> the gateway wakes on a notification -> received in ROS 2

%%{init: {"flowchart": {"subGraphTitleMargin": {"top": 10, "bottom": 8}}} }%%
flowchart LR
    subgraph ros2a["ROS 2"]
        pub["ros2 topic pub"]:::external
    end

    gw1["Gateway"]:::gateway

    subgraph iox2["iceoryx2"]
        lim["Twist Limiter"]
    end

    gw2["Gateway"]:::gateway

    subgraph ros2b["ROS 2"]
        echo["ros2 topic echo"]:::external
    end

    pub -- "/cmd_vel<br/>DDS" --> gw1
    gw1 -- "Twist<br/>SHM" --> lim
    lim -- "Twist<br/>SHM" --> gw2
    gw2 -- "/cmd_vel_limited<br/>DDS" --> echo
```

Along the way, each component wakes on its own event:

1. The gateway wakes when a message arrives from ROS 2.
1. The limiter wakes when the gateway delivers a sample to `CmdVel`.
1. The gateway wakes when the limiter notifies `CmdVelLimited`.

## Setting Up

Let's build on the setup from
[Plain Struct as Payload](/tutorials/gateway-to-ros-2/plain-struct-as-payload).
We create a new `cargo` project for the event-driven limiter in the same working
directory:

```text
~/iceoryx2_ros2/
├── shouter/                      # cargo project from a previous article
├── twist_limiter/                # cargo project from a previous article
└── twist_limiter_event_driven/   # cargo project of the iceoryx2 application
```

```console
cd ~/iceoryx2_ros2
cargo new twist_limiter_event_driven
cd twist_limiter_event_driven
```

The `Cargo.toml` is the same as the one of the polling limiter:

```{code-block} toml
:caption: twist_limiter_event_driven/Cargo.toml

[package]
name = "twist_limiter_event_driven"
edition = "2024"
publish = false

[dependencies]
iceoryx2 = { version = "X.Y.Z" } # select the desired `iceoryx2` version
# iceoryx2 = { git = "https://github.com/eclipse-iceoryx/iceoryx2", branch = "main" } # if the gateway is built from `main`
ros-env = { version = "0.2" }
rosidl_runtime_rs = { version = "0.7" } # the version `ros-env` depends on
```

As before, the `iceoryx2` version must match the one the gateway is built
from.

## The Twist Limiter

The event-driven limiter will consist of the same payload module as the polling
limiter and a new application:

```text
~/iceoryx2_ros2/twist_limiter_event_driven/
├── Cargo.toml
├── mapping.toml   # gateway configuration, see below
└── src/
    ├── main.rs    # the application
    └── twist.rs   # the payload type
```

### The Payload

The payload type is the same as the one of the
[polling limiter](/tutorials/gateway-to-ros-2/plain-struct-as-payload.md#the-payload).
Create the module for the payload type:

```console
cd ~/iceoryx2_ros2/twist_limiter_event_driven
touch src/twist.rs
```

And add the wrapper to it:

```{literalinclude} ../../../snippets/gateway-to-ros-2/twist_limiter_event_driven/src/twist.rs
:language: rust
:caption: twist_limiter_event_driven/src/twist.rs
```

### The Application

The application will be similar, except that it additionally waits on the
event service matching the service name of its `CmdVel` subscriber so that it
wakes as soon as the gateway delivers a sample. After each publish, it also
notifies the event service matching the service name of its `CmdVelLimited`
publisher to wake the gateway:

```{literalinclude} ../../../snippets/gateway-to-ros-2/twist_limiter_event_driven/src/main.rs
:language: rust
:caption: twist_limiter_event_driven/src/main.rs
```

## The Gateway

The gateway uses the same mapping and translator as for the polling limiter.
It differs in when it runs. Instead of polling, it wakes on data arriving from
ROS 2 and on notifications from the limiter. It also wakes the limiter by
notifying it whenever it delivers a sample to `CmdVel`.

### Mapping

The mapping is the same as the one of the
[polling limiter](/tutorials/gateway-to-ros-2/plain-struct-as-payload.md#mapping).
Create the configuration file in the project directory:

```console
cd ~/iceoryx2_ros2/twist_limiter_event_driven
touch mapping.toml
```

And add the mappings to it:

```{code-block} toml
:caption: twist_limiter_event_driven/mapping.toml

[[mapping]]
iceoryx2.service_name = "CmdVel"
iceoryx2.payload_type = "geometry_msgs/msg/Twist"
ros2.topic = "/cmd_vel"
ros2.type = "geometry_msgs/msg/Twist"

[[mapping]]
iceoryx2.service_name = "CmdVelLimited"
iceoryx2.payload_type = "geometry_msgs/msg/Twist"
ros2.topic = "/cmd_vel_limited"
ros2.type = "geometry_msgs/msg/Twist"
```

### Waking

Three options make the gateway event-driven:

* `--reactive` wakes the gateway when data arrives over DDS or when the ROS 2
  endpoints change.
* `--notify` makes the gateway notify the `CmdVel` event service whenever it
  publishes a message from the `/cmd_vel` topic on the `CmdVel`
  publish-subscribe service.
* `--listener CmdVelLimited` wakes the gateway whenever the limiter notifies
  the `CmdVelLimited` event service.

```{note}
Polling is disabled when `--reactive` or `--listener` is given. It can be
re-enabled alongside the other options by setting `--poll` explicitly.
```

## Running

Now with all pieces implemented and configured, we can run the complete
pipeline. Each application will run in a separate terminal and requires the
ROS 2 distribution to be sourced.

First, launch the limiter:

```console
source /opt/ros/<distro>/setup.bash
# source ~/iceoryx2_ros2/messages/install/setup.bash  # on older installations
cd ~/iceoryx2_ros2/twist_limiter_event_driven
cargo run
```

Next, launch the gateway with the options from the previous section:

```console
source /opt/ros/<distro>/setup.bash
# source ~/iceoryx2_ros2/messages/install/setup.bash  # on older installations
cd ~/iceoryx2_ros2/twist_limiter_event_driven
iox2 link gateway ros2 --static-mapping mapping.toml --translator PlainStruct \
    --reactive --notify --listener CmdVelLimited
```

Then start observing the limited result. The type is given explicitly because
the topic is not yet published. Since shared memory communication guarantees
delivery, the subscription requests reliable delivery to match:

```console
source /opt/ros/<distro>/setup.bash
# source ~/iceoryx2_ros2/messages/install/setup.bash  # on older installations
ros2 topic echo --qos-reliability reliable /cmd_vel_limited geometry_msgs/msg/Twist
```

Finally, publish velocity commands at 1 Hz that exceed the configured
maximum:

```console
source /opt/ros/<distro>/setup.bash
# source ~/iceoryx2_ros2/messages/install/setup.bash  # on older installations
ros2 topic pub -r 1 /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 5.0}}"
```

The output is the same as with the polling limiter:

```console
$ cargo run
limited linear.x from 5 to 1
limited linear.x from 5 to 1
limited linear.x from 5 to 1
```

```console
$ ros2 topic echo --qos-reliability reliable /cmd_vel_limited geometry_msgs/msg/Twist
linear:
  x: 1.0
  y: 0.0
  z: 0.0
angular:
  x: 0.0
  y: 0.0
  z: 0.0
---
linear:
  x: 1.0
  y: 0.0
  z: 0.0
angular:
  x: 0.0
  y: 0.0
  z: 0.0
---
```

This time, every command moves through the pipeline as soon as it is
published. Between two commands, neither the gateway nor the limiter wakes.

## Conclusion

An event-driven pipeline has no polling latency and lets idle components sleep.
In exchange, every application must handle events. Each publisher notifies after
sending, and the gateway needs one `--listener` per service it forwards to
ROS 2.

Polling remains the simpler choice when its latency is acceptable or when the
applications cannot be changed. If desired, polling can be enabled with
`--poll` in addition to these options, which may be useful when not every
application notifies after publishing.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Event-Driven Communication
:link: /getting-started/robot-nervous-system/event-driven-communication
:link-type: doc
:shadow: none

Wake applications with events instead of polling for samples.
```

```{grid-item-card} Build the ROS 2 gateway
:link: /how-to/build-ros-2-gateway
:link-type: doc
:shadow: none

Build the gateway against your own ROS 2 workspace.
```

```{grid-item-card} Links: Tunnels and Gateways
:link: /fundamentals/links
:link-type: doc
:shadow: none

Understand how to extend `iceoryx2` beyond its shared memory domain.
```

````
