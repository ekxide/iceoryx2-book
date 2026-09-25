# Gateway Basics

```{important}
The ROS 2 integrations are currently prototypes and have not yet been validated
in real ROS 2 workflows. They are recommended only for experimentation in
development deployments.
```

Before diving into possible gateway configurations, let's first go over some
gateway basics.

The gateway component sits at the boundary of `iceoryx2` and another
communication mechanism, in this case ROS 2, and propagates payloads
across it. Inbound data gets written into shared memory, after which it is
shared zero-copy between `iceoryx2` nodes. Likewise, outbound data
is provided by nodes to the gateway without copies.

The gateway can be either [embedded into your own process](
https://github.com/eclipse-iceoryx/iceoryx2/tree/main/integrations/ros2/link-adapter#usage
), allowing you to
have full control over its execution, or run in an isolated process via the
`iox2` CLI.

## Building

The gateway must be built from source against a ROS 2 install space. This is
because it must link against the specific `rcl` library being used, as well as
any generated message libraries. See [Build ROS 2 Integration Crates](
/how-to/build-ros-2-integration-crates) for details on how to build the
gateway.

```{important}
A sources ROS 2 workspace is also required to run the gateway as it is
required to load typesupport libraries.
```

## Gateway Configuration

There are two main capabilities that should be understood in order to be able
to choose the right configuration for your system. The mapping, which
associates `iceoryx2` services with the endpoints of the counterpart
communication mechanism, and the translation, which optionally processes
payloads as they cross the boundary between them.

### Mapping

`PrefixMapping` is the default strategy. It derives pairings between ROS 2
topics and `iceoryx2` services from a naming convention. All service names
matching the convention are propagated with best-guess quality of service:

```text
ros2://topics/{NAMESPACE}/{TOPIC}  <->  /{NAMESPACE}/{TOPIC}
     (iceoryx2 service name)             (ROS 2 topic name)
```

The topics can be filtered by explicitly specifying allowed topics. When this
`--allow` is used, only those specified topics will be propagated:

```console
iox2 link gateway ros2 --allow "/cmd_vel" --allow "/sensors/*"
```

The gateway will attempt to dynamically load typesupport for discovered topics,
skipping those it is unable to load. This approach is only recommended as a
starting point when first configuring the system.

`StaticMapping` declares pairings explicitly in a TOML file. Only the specified
pairings are propagated and their types are resolved immediately at startup,
which enables fast failure on misconfiguration. The `iceoryx2` service
settings and ROS 2 QoS can also be explicitly configured for each entry. See
the [example configuration](
https://github.com/eclipse-iceoryx/iceoryx2/blob/main/integrations/ros2/link-gateway-cli/static-mapping.example.toml)
for available configuration options.

A minimal entry pairs one service with one topic:

```{code-block} toml
:caption: mapping.toml

[[mapping]]
iceoryx2.service_name = "Chatter"
iceoryx2.payload_type = "std_msgs/msg/String"
ros2.topic = "/chatter"
ros2.type = "std_msgs/msg/String"
```

The mapping file is passed on launch:

```console
iox2 link gateway ros2 --static-mapping mapping.toml
```

This approach is recommended once the shape of a system is understood.

### Translation

`Passthrough` moves payload bytes across the boundary unmodified and is the
default. The payload must be a byte slice holding the CDR serialization of the
ROS 2 message of the paired topic. It is left to the applications to
(de)serialize the bytes.

`PlainStruct` (de)serializes payloads at the boundary using the ROS 2
typesupport libraries. The `iceoryx2` applications work directly with the
plain struct in shared memory, while the gateway converts to and from the
CDR bytes that ROS 2 expects. Only self-contained types that can be stored
directly in shared memory are supported. It is selected on launch:

```console
iox2 link gateway ros2 --translator PlainStruct
```

## Application Configuration

Applications require minimal changes to integrate with the ROS 2 gateway.
The gateway automatically connects to services mapped by the selected mapping
and propagates payloads without any involvement of the applications.

There is only one thing that applications interfacing with the ROS 2 gateway
must do: the payload type name of a propagated service must be the ROS 2 type
name of the paired topic, for example `geometry_msgs/msg/Twist`. The gateway
resolves the typesupport used for translation by this name. When translating
to plain structs, the payload's size and alignment are additionally verified
against the layout of the ROS 2 type, and the service is not propagated when
they contradict.

The type name can be specified on the payload type when implementing
`ZeroCopySend` on payload types:

```rust
#[derive(ZeroCopySend)]
#[type_name("std_msgs/msg/Float64")]
#[repr(C)]
pub struct Payload {
    pub data: f64,
}
```

For ROS 2 types generated for Rust this is typically set by wrapping them
in a new type and implementing te trait:

```rust
use rosidl_runtime_rs::{Message, RmwMessage};

#[derive(Debug, Default, Clone)]
#[repr(transparent)]
pub struct Float64(pub std_msgs::msg::rmw::Float64);

unsafe impl ZeroCopySend for Float64 {
    unsafe fn type_name() -> &'static str {
        <<std_msgs::msg::Float64 as Message>::RmwMsg as RmwMessage>::TYPE_NAME
    }
}
```

Delegating to the generated `TYPE_NAME` constant is preferred over hardcoding
the name, as a typo in a hardcoded name does not fail at compile time but
silently prevents the service from being propagated.

### Message Info

ROS 2 delivers each message with its message info, such as the writer that
published it, its source timestamp and its sequence number. By default, the
message info is dropped at the boundary and the services the gateway creates
for topics have no user header.

Applications that want to read the message info declare the `RosHeader` as the
user header of the service:

```rust
use iceoryx2_integrations_ros2_interop::RosHeader;

let service = node
    .service_builder(&"CmdVel".try_into()?)
    .publish_subscribe::<Payload>()
    .user_header::<RosHeader>()
    .open_or_create()?;
```

If the header is specified, the gateway must be launched with `--ros-header` so
that it also creates services mirroring topics with the header.

```console
iox2 link gateway ros2 --ros-header
```

When ingesting messages from ROS 2, the gateway fills this header with the
message info, which subscribers may use to identify the remote writer or
detect message loss. Publishing applications can leave it at its default as
ROS 2 populates it automatically.

A service with any other user header is not propagated.

## Running

The gateway discovers matching services and topics and propagates pending
samples whenever it wakes. How it wakes is configurable.

By default, the gateway polls every 100 milliseconds. A different polling
rate (in milliseconds) can be set with `--poll`:

```console
iox2 link gateway ros2 --poll 10
```

The gateway can also wake reactively. With `--reactive` it wakes
whenever data arrives or endpoints change on the ROS 2 side. With `--listener`, which is
repeatable, it wakes whenever the named `iceoryx2` event service fires. When
either is given, polling becomes opt-in and is only enabled when `--poll` is
set explicitly:

```console
iox2 link gateway ros2 --reactive --listener "SensorData"
```

When embedding the gateway, it is driven by calling `discover()` and
`propagate()` on the link, either on a timer or whenever the listener it
provides wakes, as shown in [its usage examples](
https://github.com/eclipse-iceoryx/iceoryx2/tree/main/integrations/ros2/link-adapter#usage).

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Build ROS 2 Integration Crates
:link: /how-to/build-ros-2-integration-crates
:link-type: doc
:shadow: none

Build the gateway and the `ament_cargo` packages against your own ROS 2
workspace.
```

```{grid-item-card} Example Nodes
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/integrations/ros2/workspace/src/demo_nodes
:link-type: url
:shadow: none

Reference applications for each combination of service-to-topic mapping and
payload translator.
```

```{grid-item-card} Links: Tunnels and Gateways
:link: /fundamentals/links
:link-type: doc
:shadow: none

Understand how to extend `iceoryx2` beyond its shared memory domain.
```

````
