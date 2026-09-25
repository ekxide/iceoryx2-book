# Plain Struct as Payload

```{important}
The ROS 2 integrations are currently prototypes and have not yet been validated
in real ROS 2 workflows. They are recommended only for experimentation in
development deployments.
```

In the [ROS 2 interface description language (IDL)](
https://docs.ros.org/en/rolling/ROS-Framework/interfaces/About-Interfaces.html)
the types of the message fields dictate the native types generated.
If a definition does not contain any bounded or dynamic fields, the generated
native types are completely self-contained and can thus be placed in shared
memory.

```text
float64 scalar            # fixed-width basic type, shared-memory compatible
float64[3] fixed_array    # fixed-size array, shared-memory compatible
float64[<=8] bounded      # bounded sequence, heap-backed
float64[] dynamic         # dynamic sequence, heap-backed
string text               # dynamic string, heap-backed
```

For ROS 2 topics whose message definitions are free of bounded and dynamic
fields, a convenient approach is to let the gateway's `PlainStruct`
translator (de)serialize messages to CDR as they cross the boundary between
ROS 2 and `iceoryx2`, and store the plain struct directly in shared memory
to be consumed by `iceoryx2` applications. The (de)serialization is done once
after which the payload is shared without copies.

## Topology

To demonstrate the approach, let's build a minimal pipeline that carries a
self-contained message through both domains. A limiter application in
`iceoryx2` that subscribes to the `/cmd_vel` topic, clamps the received
velocity to a maximum, and republishes it on `/cmd_vel_limited`.

Both topics use
[`geometry_msgs/msg/Twist`](https://github.com/ros2/common_interfaces/blob/rolling/geometry_msgs/msg/Twist.msg),
which consists of two `Vector3`s of fixed-width floats and contains no bounded
or dynamic fields.

```{mermaid}
:caption: The Twist message crossing the boundary between ROS 2 and iceoryx2
:alt: ROS 2 publishes Twist -> received, transformed and re-published in iceoryx2 -> received in ROS 2

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

To keep things simple, the `ros2cli` is used to mock both ends, with
`ros2 topic pub` feeding commands and `ros2 topic echo` displaying the
limited result.

## Setting Up

```{note}
The integrations are verified against Jazzy with `rmw_fastrtps_cpp` and Humble
with `rmw_cyclonedds_cpp`. All commands below assume a sourced ROS 2
environment, where `<distro>` is your distribution.
```

In our pipeline, the `Twist` message describes the payload that crosses
the boundary between the two domains. From the one message definition, the
types for both the C/C++/Python ROS 2 side and the Rust `iceoryx2` side can
be generated.

The Rust types are generated in a `colcon` workspace, while the `iceoryx2`
application is a regular `cargo` project that uses them. Both live side by
side in one directory:

```text
~/iceoryx2_ros2/
├── messages/        # colcon workspace generating the Rust message crates
└── twist_limiter/   # cargo project of the iceoryx2 application
```

### Message Generation

Firstly, we create a `colcon` workspace for our message definitions:

```console
mkdir -p ~/iceoryx2_ros2/messages/src
cd ~/iceoryx2_ros2/messages
```

Next we will need to pull in some third-party packages. We will require
`rosidl_generator_rs` which emits Rust types for our message definitions from
the `rosidl_rust` repository. We will also require the packages that provide
the message definitions. The `Twist` message is part of `common_interfaces`,
which has a dependency to `builtin_interfaces` from the `rcl_interfaces`
repository.

As conventional for ROS 2, `vcstool` is used here to pull in the source. Here is
the `.repos` configuration. Be sure to substitute `<distro>` with the ROS 2
distro being used (e.g. `jazzy`):

```{code-block} yaml
:caption: \<distro\>.repos

repositories:
  rosidl_rust:
    type: git
    url: https://github.com/ros2-rust/rosidl_rust.git
    version: 0.5.0
  common_interfaces:
    type: git
    url: https://github.com/ros2/common_interfaces.git
    version: <distro>
  rcl_interfaces:
    type: git
    url: https://github.com/ros2/rcl_interfaces.git
    version: <distro>
```

The source can then be pulled with into the workspace:

```console
pip install vcstool
vcs import src < <distro>.repos
```

```{note}
ROS 2 distributions ship C, C++ and Python types for the common message
definitions, including `Twist`. At the time of writing this article, no
distribution ships with generated Rust types out-of-the-box.
```

Finally, building the generator and the message definitions generates the
message types:

```console
colcon build --packages-up-to std_msgs geometry_msgs rosidl_generator_rs
```

Alongside the usual C, C++ and Python artifacts, each message package installs
a Rust crate under `install/<package>/share/<package>/rust`. Sourcing the
install space makes these crates available to `cargo` projects:

```console
source ~/iceoryx2_ros2/messages/install/setup.bash
```

### Application Project

Now let's create a `cargo` project for the `iceoryx2` application in our
pipeline, next to the message workspace:

```console
cd ~/iceoryx2_ros2
cargo new twist_limiter
cd twist_limiter
```

The generated message crates are included through the [`ros-env`](
https://github.com/ros2-rust/ros-env) crate, which provides the message
crates of the sourced environment under its root, e.g.
`ros_env::geometry_msgs`. The version of `rosidl_runtime_rs` must match the
one `ros-env` builds the message crates with:

```{code-block} toml
:caption: twist_limiter/Cargo.toml

[package]
name = "twist_limiter"
edition = "2024"
publish = false

[dependencies]
iceoryx2 = { version = "X.Y.Z" } # select the desired `iceoryx2` version
ros-env = { version = "0.2" }
rosidl_runtime_rs = { version = "0.7" }
```

Now that everything is in place, let's create a basic placeholder binary to
confirm the build works and that the generated messages are accessible.

The generated crates contain [two variants of each message](
https://docs.rs/rosidl_runtime_rs/latest/rosidl_runtime_rs/trait.Message.html).
The idiomatic `geometry_msgs::msg::Twist` uses native Rust types, while its
counterpart in the `rmw` module matches the layout of the equivalent C struct
through `#[repr(C)]`. Payloads in shared memory are read as raw bytes
across processes, which requires this consistent layout, so the `rmw` variant
must be used:

```{literalinclude} ../../../snippets/gateway-to-ros-2/twist_limiter/src/bin/placeholder.rs
:language: rust
:caption: twist_limiter/src/main.rs
:start-after: snippet:start placeholder
:end-before: snippet:end placeholder
```

The generated crates link against the C libraries of their message packages,
so the install space of the message workspace must be sourced before building
the project:

```console
source ~/iceoryx2_ros2/messages/install/setup.bash
cargo build
```

The libraries are also loaded when the binary starts, so it is run from the
same shell:

```console
$ cargo run
Twist { linear: Vector3 { x: 0.0, y: 0.0, z: 0.0 }, angular: Vector3 { x: 0.0, y: 0.0, z: 0.0 } }
```

A successful build and run, with the above output, confirms that the generated
Rust types are reachable and the project is set up properly.

## The Twist Limiter

With the project skeleton set up, let's implement the limiter.

The generated `Twist` type needs some boilerplate for it to be used with
`iceoryx2`. First we must implement the `ZeroCopySend` trait to indicate that
the type can be stored in shared memory. The type name it reports must be
the ROS 2 type name, as described in [Application Configuration](
/tutorials/gateway-to-ros-2/gateway-basics.md#application-configuration).
This name is exposed in the generate Rust message crate:

```{literalinclude} ../../../snippets/gateway-to-ros-2/twist_limiter/src/main.rs
:language: rust
:caption: twist_limiter/src/main.rs
:start-after: snippet:start payload
:end-before: snippet:end payload
```

Replacing the placeholder `main`, the application itself is hardly any
different from a regular publish-subscribe application. The only thing
ROS-specific is the `Twist` payload type defined above:

```{literalinclude} ../../../snippets/gateway-to-ros-2/twist_limiter/src/main.rs
:language: rust
:caption: twist_limiter/src/main.rs
:start-after: snippet:start limiter
:end-before: snippet:end limiter
```

The limiting is just a basic clamp on the velocity. This same shape however
is representative for any real application which is likely to be more complex.
Note that the payload is a plain struct on both services. At no point does the
application (de)serialize anything, as it is completely handled by the
gateway.

## The Gateway

The final component is the gateway itself, which connects the two topics to
their `iceoryx2` counterparts. Refer to [Gateway Basics](
/tutorials/gateway-to-ros-2/gateway-basics.md) for an overview of the
configuration options. Here we will
only look at the configuration suitable for this pipeline.

### Mapping

To associate the topics in ROS 2 with the services in `iceoryx2`, we will
use static mapping, which can be defined with a configuration file:

```{code-block} toml
:caption: twist_limiter/mapping.toml

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

The `payload_type` matches the type name reported by the `Twist` wrapper
defined earlier.

### Translator

For self-contained plain structs, the `PlainStruct` translator provides the
(de)serialization at the boundary that this approach relies on.

From the `twist_limiter` directory, the gateway is launched with:

```console
iox2 link gateway ros2 --static-mapping mapping.toml --translator PlainStruct
```

## Running

Now with all pieces implemented and configured, we can run the complete
pipeline. Each application will run in a separate terminal and requires the
install space of the message workspace to be sourced.

First, launch the limiter:

```console
source ~/iceoryx2_ros2/messages/install/setup.bash
cd ~/iceoryx2_ros2/twist_limiter
cargo run
```

Next, launch the gateway with the configuration from the previous section:

```console
source ~/iceoryx2_ros2/messages/install/setup.bash
cd ~/iceoryx2_ros2/twist_limiter
iox2 link gateway ros2 --static-mapping mapping.toml --translator PlainStruct
```

Finally, publish velocity commands at 1 Hz that exceed the configured
maximum, and observe the limited result:

```console
source ~/iceoryx2_ros2/messages/install/setup.bash
ros2 topic pub -r 1 /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 5.0}}"
```

```console
source ~/iceoryx2_ros2/messages/install/setup.bash
ros2 topic echo /cmd_vel_limited
```

Every published message is received by the gateway over DDS and deserialized
directly into shared memory, where the limiter reads and writes plain
structs. On the way out, the gateway serializes the limited message to CDR
and publishes it on the ROS 2 topic:

```console
$ cargo run
limited linear.x from 5 to 1
limited linear.x from 5 to 1
limited linear.x from 5 to 1
```

```console
$ ros2 topic echo /cmd_vel_limited
linear:
  x: 1.0
  y: 0.0
  z: 0.0
angular:
  x: 0.0
  y: 0.0
  z: 0.0
```

## Conclusion

The approach described in this article is a good fit whenever the message
definitions crossing the boundary are self-contained. The payload is
(de)serialized only at the gateway crossings, and every application behind
them reads a plain struct straight from shared memory.

For message definitions with bounded or dynamic fields,
[FlatBuffer as Payload](/tutorials/gateway-to-ros-2/flatbuffer-as-payload)
provides a shared-memory-compatible representation. If the gateway should stay
out of payload translation entirely,
[CDR Bytes as Payload](/tutorials/gateway-to-ros-2/cdr-bytes-as-payload)
forwards the serialized bytes unmodified.

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
