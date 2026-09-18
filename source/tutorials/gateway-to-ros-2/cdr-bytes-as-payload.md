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

One way to bridge these types anyway is to keep them in their serialized
form, which in ROS 2 is typically the Common Data Representation (CDR) used
by DDS. This can be done by utilizing the `Passthrough` translator, which
moves the serialized bytes across the boundary unmodified, in both
directions.

With this approach, every message type becomes bridgeable and the gateway
performs no payload processing at all. The trade-off is that applications
take on the (de)serialization responsibility and the overhead.

## Topology

To demonstrate the approach, let's build a minimal pipeline that carries a
message containing a dynamic field through both domains. A relay application in
`iceoryx2` that subscribes to the `/chatter` topic, uppercases the received
text, and republishes it on `/chatter_upper`.

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
        rly["Chatter Relay"]
    end

    gw2["Gateway"]:::gateway

    subgraph ros2b["ROS 2"]
        echo["ros2 topic echo"]:::external
    end

    pub -- "/chatter<br/>DDS" --> gw1
    gw1 -- "CDR bytes<br/>SHM" --> rly
    rly -- "CDR bytes<br/>SHM" --> gw2
    gw2 -- "/chatter_upper<br/>DDS" --> echo
```

The message crosses the boundary twice with the gateway storing the CDR
bytes directly in shared memory. In this case, the `iceoryx2` application
must take on the responsibility of (de)serialization.

To keep things simple, the `ros2cli` is used to mock both ends, with
`ros2 topic pub` feeding text and `ros2 topic echo` displaying the
uppercased result.

## Setting Up

Let's build on the `colcon` workspace set up in
[Plain Struct as Payload](/tutorials/gateway-to-ros-2/plain-struct-as-payload.md).
The `String` message is part of `std_msgs`, which was already generated
there, so no additional message generation is needed. If starting fresh here,
follow the package setup and message generation steps in the previous article.

We create a new package for our string relay, set up in the same way as
described in the previous article:

```console
mkdir -p src/chatter_relay/src
```

```{code-block} xml
:caption: src/chatter_relay/package.xml

<?xml version="1.0"?>
<package format="3">
    <name>chatter_relay</name>
    <version>0.1.0</version>
    <description>Chatter messages uppercased in iceoryx2</description>
    <maintainer email="you@example.com">you</maintainer>
    <license>Apache 2.0</license>

    <depend>std_msgs</depend>

    <export>
        <build_type>ament_cargo</build_type>
    </export>
</package>
```

```{code-block} rust
:caption: src/chatter_relay/build.rs

fn main() {
    let prefix_path = std::env::var("AMENT_PREFIX_PATH")
        .expect("AMENT_PREFIX_PATH not set - source the ROS 2 workspace before building");
    for prefix in prefix_path.split(':') {
        let lib = format!("{prefix}/lib");
        println!("cargo:rustc-link-search=native={lib}");
        println!("cargo:rustc-link-arg=-Wl,-rpath,{lib}");
    }
}
```

There are two notable differences in the `Cargo.toml`.
The `cdr` crate is added as an additional dependency, and the `serde` feature
is enabled on the message crate to make the generated types (de)serializable:

```{code-block} toml
:caption: src/chatter_relay/Cargo.toml

[workspace]

[package]
name = "chatter_relay"
edition = "2024"
publish = false

[dependencies]
cdr = { version = "0.2" }
iceoryx2 = { version = "X.Y.Z" } # select the desired `iceoryx2` version
iceoryx2-integrations-ros2-interop = { version = "X.Y.Z" } # same version as `iceoryx2`
rosidl_runtime_rs = { version = "0.6" }
std_msgs = { version = "*", features = ["serde"] }
```

## The Chatter Relay

For this approach, the payload type is opaque CDR-serialized bytes. However,
the type name also needs to be set so the bytes can be properly associated
with the type:

```{code-block} rust
:caption: src/chatter_relay/src/main.rs

use iceoryx2::prelude::*;
use rosidl_runtime_rs::{Message, RmwMessage};

#[derive(Debug, Clone, Copy)]
#[repr(transparent)]
pub struct StringByte(pub u8);

unsafe impl ZeroCopySend for StringByte {
    unsafe fn type_name() -> &'static str {
        <<std_msgs::msg::String as Message>::RmwMsg as RmwMessage>::TYPE_NAME
    }
}
```

Now let's implement the application. The payload type is declared as a slice
of `StringByte` in both directions and the `RosHeader` is specified as the
user header since both services are bridged with ROS 2. For dynamic
message types the size of the message is not known at compile time, so the
publisher is configured with an initial size guess and an
allocation strategy to grow the shared memory when required:

```{code-block} rust
:caption: src/chatter_relay/src/main.rs

use core::time::Duration;

use cdr::{CdrLe, Infinite};
use iceoryx2::prelude::*;
use iceoryx2_integrations_ros2_interop::RosHeader;

const CYCLE_TIME: Duration = Duration::from_millis(100);
const INITIAL_MAX_PAYLOAD_SIZE: usize = 64;

fn main() -> Result<(), Box<dyn core::error::Error>> {
    set_log_level_from_env_or(LogLevel::Info);

    let node = NodeBuilder::new().create::<ipc::Service>()?;

    let chatter = node
        .service_builder(&"Chatter".try_into()?)
        .publish_subscribe::<[StringByte]>()
        // IMPORTANT: Must use this user header if crossing ROS 2 boundary.
        .user_header::<RosHeader>()
        .open_or_create()?;
    let subscriber = chatter.subscriber_builder().create()?;

    let chatter_upper = node
        .service_builder(&"ChatterUpper".try_into()?)
        .publish_subscribe::<[StringByte]>()
        // IMPORTANT: Must use this user header if crossing ROS 2 boundary.
        .user_header::<RosHeader>()
        .open_or_create()?;
    let publisher = chatter_upper
        .publisher_builder()
        .initial_max_slice_len(INITIAL_MAX_PAYLOAD_SIZE)
        .allocation_strategy(AllocationStrategy::PowerOfTwo)
        .create()?;

    while node.wait(CYCLE_TIME).is_ok() {
        while let Some(sample) = subscriber.receive()? {
            let bytes: Vec<u8> =
                sample.payload().iter().map(|byte| byte.0).collect();

            // The application must deserialize the CDR bytes itself
            let mut message: std_msgs::msg::String = cdr::deserialize(&bytes)?;
            message.data = message.data.to_uppercase();

            // And then serialize the outgoing data back to CDR
            let payload = cdr::serialize::<_, _, CdrLe>(&message, Infinite)?;
            let upper_sample = publisher.loan_slice_uninit(payload.len())?;
            let upper_sample =
                upper_sample.write_from_fn(|index| StringByte(payload[index]));
            upper_sample.send()?;

            coutln!(
                "relayed \"{}\" ({} bytes, sequence {})",
                message.data,
                payload.len(),
                sample.user_header().sequence_number
            );
        }
    }

    Ok(())
}
```

Each received sample is deserialized into the generated `String` type,
transformed, and serialized back into a loaned slice of matching length.
Unlike in the plain struct article, the (de)serialization happens in the
application, while the gateway only moves bytes.

## The Gateway

The final component is the gateway itself, which bridges the two topics to
their `iceoryx2` counterparts. Refer to [Gateway Basics](
/tutorials/gateway-to-ros-2/gateway-basics.md) for an overview. Here we will
only look at the configuration suitable for this pipeline.

### Mapping

To associate the topics in ROS 2 with the services in `iceoryx2`, we will
use static mapping, which can be defined with a configuration file:

```{code-block} toml
:caption: mapping.toml

[[mapping]]
iceoryx2.service_name = "Chatter"
iceoryx2.payload_type = "std_msgs/msg/String"
ros2.topic = "/chatter"
ros2.type = "std_msgs/msg/String"

[[mapping]]
iceoryx2.service_name = "ChatterUpper"
iceoryx2.payload_type = "std_msgs/msg/String"
ros2.topic = "/chatter_upper"
ros2.type = "std_msgs/msg/String"
```

The `payload_type` matches the type name reported by the `StringByte` type
defined earlier.

### Translator

For payloads crossing as serialized bytes, the `Passthrough` translator is
the correct choice. It is the default, so specifying it explicitly is
optional.

With the mapping file in the workspace root, the gateway is launched with:

```console
iox2 gateway ros2 --static-mapping mapping.toml --translator Passthrough
```

## Running

Now with all pieces implemented and configured, we can run the complete
pipeline. Each application will run in a separate terminal and requires
the install space to be sourced.

First, build the package so that the relay is installed in the install
space:

```console
colcon build --packages-select chatter_relay
```

Then launch the relay:

```console
source install/setup.bash
ros2 run chatter_relay chatter_relay
```

Next, launch the gateway with the configuration from the previous section:

```console
source install/setup.bash
iox2 gateway ros2 --static-mapping mapping.toml --translator Passthrough
```

Finally, publish text at 1 Hz and observe the output:

```console
source install/setup.bash
ros2 topic pub -r 1 /chatter std_msgs/msg/String "{data: hello}"
```

```console
source install/setup.bash
ros2 topic echo /chatter_upper
```

Every published message is forwarded by the gateway into shared memory as
CDR bytes, where the relay application deserializes, transforms and serializes
it. On the way out, the gateway forwards the bytes into ROS 2 unmodified:

```console
$ ros2 run chatter_relay chatter_relay
relayed "HELLO" (14 bytes, sequence 1)
relayed "HELLO" (14 bytes, sequence 2)
relayed "HELLO" (14 bytes, sequence 3)
```

```console
$ ros2 topic echo /chatter_upper
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
When the (de)serialization code itself must be certified, each application
owns and certifies its own, while the gateway never touches payload
contents and stays out of the certified path.

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

```{grid-item-card} Understand Gateways and Tunnels
:link: /fundamentals/gateways-and-tunnels
:link-type: doc
:shadow: none

Get familiar with how `iceoryx2` extends communication beyond a single host.
```

````
