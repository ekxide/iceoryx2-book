# CDR Bytes as Payload

```{important}
The ROS 2 integrations are currently prototypes and have not yet been validated
in real ROS 2 workflows. They are recommended only for experimentation in
development deployments.
```

Many standard ROS 2 message definitions contain bounded or dynamic fields. As
discussed in [Plain Struct as Payload](/tutorials/gateway-to-ros-2/plain-struct-as-payload.md),
the native types
generated for such definitions utilize the heap and cannot be placed in
shared memory.

One way to propagate these types anyway is to keep them in their serialized
form, which in ROS 2 is typically the Common Data Representation (CDR) used
by DDS. This can be done by utilizing the `Passthrough` translator, which
moves the serialized bytes across the boundary unmodified, in both
directions.

With this approach, every message type can be propagated and the gateway
performs no payload processing at all. The trade-off is that applications
take on the (de)serialization responsibility and the overhead.

Independent of the message type, keeping the (de)serialization in the
application can also be desirable, for example when it must be certified as
part of the application, or isolated from the processes communicating over the
network.

## Topology

To demonstrate the approach, let's build a minimal pipeline that carries a
message containing a dynamic field through both domains. A shouter application
in `iceoryx2` that subscribes to the `/chatter` topic, uppercases the received
text, and publishes it on `/shouter`.

Both topics use
[`std_msgs/msg/String`](https://github.com/ros2/common_interfaces/blob/rolling/std_msgs/msg/String.msg),
which contains a dynamic `data` field, resulting in its generated type
being unsuitable for shared memory.

```{hint}
Dynamic fields are not always obvious at a glance. Even mostly-numeric
definitions like
[`nav_msgs/msg/Odometry`](https://github.com/ros2/common_interfaces/blob/rolling/nav_msgs/msg/Odometry.msg)
are heap-backed due to strings nested in their definitions.
```

```{mermaid}
:caption: The String message crossing the boundary between ROS 2 and iceoryx2
:alt: ROS 2 publishes String -> received, transformed and re-published in iceoryx2 -> received in ROS 2

%%{init: {"flowchart": {"subGraphTitleMargin": {"top": 10, "bottom": 8}}} }%%
flowchart LR
    subgraph ros2a["ROS 2"]
        pub["ros2 topic pub"]:::external
    end

    gw1["Gateway"]:::gateway

    subgraph iox2["iceoryx2"]
        sht["Shouter"]
    end

    gw2["Gateway"]:::gateway

    subgraph ros2b["ROS 2"]
        echo["ros2 topic echo"]:::external
    end

    pub -- "/chatter<br/>DDS" --> gw1
    gw1 -- "CDR bytes<br/>SHM" --> sht
    sht -- "CDR bytes<br/>SHM" --> gw2
    gw2 -- "/shouter<br/>DDS" --> echo
```

To keep things simple, the `ros2cli` is used to mock both ends, with
`ros2 topic pub` feeding text and `ros2 topic echo` displaying the
uppercased result.

## Setting Up

Let's build on the setup from
[Plain Struct as Payload](/tutorials/gateway-to-ros-2/plain-struct-as-payload.md).
The `String` message is part of `std_msgs`, which was already generated in
the message workspace there, so no additional message generation is needed.
If starting fresh here, follow the message generation steps in the previous
article.

We create a new `cargo` project for our shouter, next to the limiter:

```console
cd ~/iceoryx2_ros2
cargo new shouter
cd shouter
```

There are two notable differences in the `Cargo.toml` compared to the previous
article. The `cdr` crate is added as an additional dependency, and the `serde`
feature is enabled on `ros-env` to make the generated types (de)serializable:

```{code-block} toml
:caption: shouter/Cargo.toml

[package]
name = "shouter"
edition = "2024"
publish = false

[dependencies]
cdr = { version = "0.2" }
iceoryx2 = { version = "X.Y.Z" } # select the desired `iceoryx2` version
ros-env = { version = "0.2", features = ["serde"] }
rosidl_runtime_rs = { version = "0.7" }
```

## The Shouter

For this approach, the payload is a slice of opaque CDR-serialized bytes.
However, the gateway still needs to know which ROS 2 message the bytes hold,
which it reads from the type name of the payload. A plain `u8` always reports
its own type name, so the bytes are wrapped in a new type that keeps the layout
of a `u8` through `#[repr(transparent)]` and reports the name of the ROS 2
message instead.

Since the new type has the layout of a `u8`, a payload can be viewed as bytes
without copying, both to read the CDR bytes it holds and to write them. The
`unsafe` views rely on this layout, so it is asserted at compile time:

```{literalinclude} ../../../snippets/gateway-to-ros-2/shouter/src/main.rs
:language: rust
:caption: shouter/src/main.rs
:start-after: snippet:start payload
:end-before: snippet:end payload
```

Now let's implement the application. The payload type is declared as a slice
of `StringByte` in both directions. For dynamic message types the size of the
message is not known at compile time, so the publisher is configured with an
initial size guess and an allocation strategy to grow the shared memory when
required:

```{literalinclude} ../../../snippets/gateway-to-ros-2/shouter/src/main.rs
:language: rust
:caption: shouter/src/main.rs
:start-after: snippet:start shouter
:end-before: snippet:end shouter
```

Each message is deserialized from and serialized into shared memory by the
application itself, while the gateway only moves bytes.

## The Gateway

The final component is the gateway itself, which connects the two topics to
their `iceoryx2` counterparts. Refer to [Gateway Basics](
/tutorials/gateway-to-ros-2/gateway-basics.md) for an overview. Here we will
only look at the configuration suitable for this pipeline.

### Mapping

To associate the topics in ROS 2 with the services in `iceoryx2`, we will
use static mapping, which can be defined with a configuration file:

```{code-block} toml
:caption: shouter/mapping.toml

[[mapping]]
iceoryx2.service_name = "Chatter"
iceoryx2.payload_type = "std_msgs/msg/String"
ros2.topic = "/chatter"
ros2.type = "std_msgs/msg/String"

[[mapping]]
iceoryx2.service_name = "Shouter"
iceoryx2.payload_type = "std_msgs/msg/String"
ros2.topic = "/shouter"
ros2.type = "std_msgs/msg/String"
```

The `payload_type` matches the type name reported by the `StringByte` type
defined earlier.

### Translator

For payloads crossing as serialized bytes, the `Passthrough` translator is
the correct choice. It is the default, so specifying it explicitly is
optional.

From the `shouter` directory, the gateway is launched with:

```console
iox2 link gateway ros2 --static-mapping mapping.toml --translator Passthrough
```

## Running

Now with all pieces implemented and configured, we can run the complete
pipeline. Each application will run in a separate terminal and requires the
install space of the message workspace to be sourced.

First, launch the shouter:

```console
source ~/iceoryx2_ros2/messages/install/setup.bash
cd ~/iceoryx2_ros2/shouter
cargo run
```

Next, launch the gateway with the configuration from the previous section:

```console
source ~/iceoryx2_ros2/messages/install/setup.bash
cd ~/iceoryx2_ros2/shouter
iox2 link gateway ros2 --static-mapping mapping.toml --translator Passthrough
```

Finally, publish text at 1 Hz and observe the output:

```console
source ~/iceoryx2_ros2/messages/install/setup.bash
ros2 topic pub -r 1 /chatter std_msgs/msg/String "{data: hello}"
```

```console
source ~/iceoryx2_ros2/messages/install/setup.bash
ros2 topic echo /shouter
```

Every published message is forwarded by the gateway into shared memory as
CDR bytes, where the shouter deserializes, transforms and serializes
it. On the way out, the gateway forwards the bytes into ROS 2 unmodified:

```console
$ cargo run
shouted "HELLO" (14 bytes)
shouted "HELLO" (14 bytes)
shouted "HELLO" (14 bytes)
```

```console
$ ros2 topic echo /shouter
data: HELLO
---
data: HELLO
---
```

## Conclusion

The approach described in this article fits every message type, including
those with bounded or dynamic fields, and keeps the gateway out of payload
translation entirely. In exchange, payloads are opaque while in transit and
every application (de)serializes them itself.

The application-side (de)serialization can also be a benefit for safety.
When the (de)serialization code itself must be certified, it is certified as
part of each application, while the gateway never touches payload contents and
stays out of the certified path.

For self-contained message definitions,
[Plain Struct as Payload](/tutorials/gateway-to-ros-2/plain-struct-as-payload)
moves the (de)serialization into the gateway, letting applications read
plain structs straight from shared memory. For a shared-memory-compatible
representation of dynamic data,
[FlatBuffer as Payload](/tutorials/gateway-to-ros-2/flatbuffer-as-payload)
is an alternative.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Build the ROS 2 Gateway
:link: /how-to/build-ros-2-gateway
:link-type: doc
:shadow: none

Build the gateway against your own ROS 2 workspace.
```

```{grid-item-card} ROS 2 Gateway Examples
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/integrations/ros2/examples
:link-type: url
:shadow: none

Example applications propagated to ROS 2 by the gateway, one set per
translator.
```

```{grid-item-card} Links: Tunnels and Gateways
:link: /fundamentals/links
:link-type: doc
:shadow: none

Understand how to extend `iceoryx2` beyond its shared memory domain.
```

````
