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

```{hint}
Bounded and dynamic fields in ROS 2 message definitions generate types that
utilize the heap.
```

For ROS 2 topics that use such message types, a convenient approach is to let
the gateway (de)serialize messages as they cross the boundary between
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

By making the `iceoryx2` application a package in a `colcon` workspace, the
`colcon` build system takes care of message generation and linkage to the
application.

### Message Generation

Firstly, we create a `colcon` workspace for our message definitions and
the application:

```console
mkdir -p iceoryx2_ros2_ws/src
cd iceoryx2_ros2_ws
```

Next we will need to pull in some third-party packages. We will require
`rosidl_generator_rs` which emits Rust types for our message definitions from
the `rosidl_rust` repository. We will also require the packages that provide
the message definitions. The `Twist` message is part of `common_interfaces`,
which has a dependency to `builtin_interfaces` from the `rcl_interfaces`
repository.

As conventional for ROS 2, `vcstool` is used here to pull in the source. Here is
the `.repos` configuration. Be sure to substitute `<distros>` with the ROS 2
distro being used (e.g. `jazzy`):

```{code-block} yaml
:caption: \<distro\>.repos

repositories:
  rosidl_rust:
    type: git
    url: https://github.com/ros2-rust/rosidl_rust.git
    version: main
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
a Rust crate under `install/<package>/share/<package>/rust`.

### Application Package

Now let's create a package for the `iceoryx2` application in our pipeline. We
will require the cargo-aware `colcon` plugins to build it:

```console
pip install colcon-cargo colcon-ros-cargo
cargo install cargo-ament-build
```

Next, we create a directory for our package. This directory will be both an
`ament` package and a `cargo` crate:

```console
mkdir -p src/twist_limiter/src
```

The `package.xml` declares `ament_cargo` as the build type to `colcon`, which
enables the `cargo` build. The message package containing `Twist` is specified
as a dependency:

```{code-block} xml
:caption: src/twist_limiter/package.xml

<?xml version="1.0"?>
<package format="3">
    <name>twist_limiter</name>
    <version>0.1.0</version>
    <description>Velocity commands limited in iceoryx2</description>
    <maintainer email="you@example.com">you</maintainer>
    <license>Apache 2.0</license>

    <depend>geometry_msgs</depend>

    <export>
        <build_type>ament_cargo</build_type>
    </export>
</package>
```

The same dependency is declared in `Cargo.toml`. The `*` version is a
placeholder which `colcon-ros-cargo` patches with the path to the message
crates in the install space of the workspace:

```{code-block} toml
:caption: src/twist_limiter/Cargo.toml

[workspace]

[package]
name = "twist_limiter"
edition = "2024"
publish = false

[dependencies]
geometry_msgs = { version = "*" }
iceoryx2 = { version = "X.Y.Z" } # select the desired `iceoryx2` version
iceoryx2-integrations-ros2-interop = { version = "X.Y.Z" } # same version as `iceoryx2`
rosidl_runtime_rs = { version = "0.6" }
```

```{hint}
The empty `[workspace]` table keeps cargo from absorbing the package into an
enclosing workspace, which would otherwise break the paths `colcon` expects.
```

We also need to point the linker at the install spaces of the sourced
environment because the generated crates link against the C libraries of their
message package, which `cargo` has no way to locate on its own:

```{code-block} rust
:caption: src/twist_limiter/build.rs

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

Now that everything is in place, let's create a basic placeholder binary to
confirm the build works and that the generated messages are accessible.

The generated crates contain [two variants of each message](
https://docs.rs/rosidl_runtime_rs/latest/rosidl_runtime_rs/trait.Message.html).
The idiomatic `geometry_msgs::msg::Twist` uses native Rust types, while its
counterpart in the `rmw` module matches the layout of the equivalent C struct.
Payloads in shared memory are read as raw bytes across processes, so the
`rmw` variant must be used:

```{code-block} rust
:caption: src/twist_limiter/src/main.rs

use geometry_msgs::msg::rmw::Twist;

fn main() {
    println!("{:?}", Twist::default());
}
```

Build the package after sourcing the install space so that the generated
message libraries are discoverable:

```console
source install/setup.bash
colcon build --packages-select twist_limiter
```

The package can be run using `ros2 run` after sourcing the install space
once more to make the newly-built binary discoverable:

```console
$ source install/setup.bash
$ ros2 run twist_limiter twist_limiter
Twist { linear: Vector3 { x: 0.0, y: 0.0, z: 0.0 }, angular: Vector3 { x: 0.0, y: 0.0, z: 0.0 } }
```

A successful build and run, with the above output, confirms that the generated
Rust types are reachable and the workspace is set up properly.

## The Twist Limiter

With the project skeleton set up, let's implement the limiter.

The generated `Twist` type needs some boilerplate for it to be used with
`iceoryx2`. First we must implement the `ZeroCopySend` trait to indicate that
the type can be stored in shared memory. The type name it reports must be
the ROS 2 type name, as described in [Application Configuration](
/tutorials/gateway-to-ros-2/gateway-basics.md#application-configuration).
This name is exposed in the generate Rust message crate:

```{code-block} rust
:caption: src/twist_limiter/src/main.rs

use iceoryx2::prelude::*;
use rosidl_runtime_rs::{Message, RmwMessage};

#[derive(Debug, Default, Clone)]
#[repr(transparent)]
pub struct Twist(pub geometry_msgs::msg::rmw::Twist);

unsafe impl ZeroCopySend for Twist {
    unsafe fn type_name() -> &'static str {
        <<geometry_msgs::msg::Twist as Message>::RmwMsg as RmwMessage>::TYPE_NAME
    }
}
```

Replacing the placeholder `main`, the application itself is hardly any
different from a regular publish-subscribe application. The only thing
ROS-specific is the `RosHeader` type declared as the user header on both
services:

```{code-block} rust
:caption: src/twist_limiter/src/main.rs

use core::time::Duration;

use iceoryx2::prelude::*;
use iceoryx2_integrations_ros2_interop::RosHeader;

const CYCLE_TIME: Duration = Duration::from_millis(100);
const MAX_VELOCITY_M_PER_S: f64 = 1.0;

fn limit(twist: &Twist) -> Twist {
    let mut limited = twist.clone();
    limited.0.linear.x = limited
        .0
        .linear
        .x
        .clamp(-MAX_VELOCITY_M_PER_S, MAX_VELOCITY_M_PER_S);
    limited
}

fn main() -> Result<(), Box<dyn core::error::Error>> {
    set_log_level_from_env_or(LogLevel::Info);

    let node = NodeBuilder::new().create::<ipc::Service>()?;

    let cmd_vel = node
        .service_builder(&"CmdVel".try_into()?)
        .publish_subscribe::<Twist>()
        // IMPORTANT: Must use this user header if crossing ROS 2 boundary.
        .user_header::<RosHeader>()
        .open_or_create()?;
    let subscriber = cmd_vel.subscriber_builder().create()?;

    let cmd_vel_limited = node
        .service_builder(&"CmdVelLimited".try_into()?)
        .publish_subscribe::<Twist>()
        // IMPORTANT: Must use this user header if crossing ROS 2 boundary.
        .user_header::<RosHeader>()
        .open_or_create()?;
    let publisher = cmd_vel_limited.publisher_builder().create()?;

    while node.wait(CYCLE_TIME).is_ok() {
        while let Some(sample) = subscriber.receive()? {
            let limited = limit(sample.payload());
            publisher.loan_uninit()?.write_payload(limited).send()?;

            coutln!(
                "limited cmd_vel (sequence {})",
                sample.user_header().sequence_number
            );
        }
    }

    Ok(())
}
```

The limiting is just a basic clamp on the velocity. This same shape however
is representative for any real application which is likely to be more complex.
Note that the payload is a plain struct on both services. At no point does the
application (de)serialize anything, as it is completely handled by the
gateway.

## The Gateway

The final component is the gateway itself, which bridges the two topics to
their `iceoryx2` counterparts. Refer to [Gateway Basics](
/tutorials/gateway-to-ros-2/gateway-basics.md) for an overview of the
configuration options. Here we will
only look at the configuration suitable for this pipeline.

### Mapping

To associate the topics in ROS 2 with the services in `iceoryx2`, we will
use static mapping, which can be defined with a configuration file:

```{code-block} toml
:caption: mapping.toml

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

With the mapping file in the workspace root, the gateway is launched with:

```console
iox2 gateway ros2 --static-mapping mapping.toml --translator PlainStruct
```

## Running

Now with all pieces implemented and configured, we can run the complete
pipeline. Each application will run in a separate terminal and requires
the install space to be sourced.

First, build the package so that the finished limiter is installed in the
install space:

```console
colcon build --packages-select twist_limiter
```

Then launch the limiter:

```console
source install/setup.bash
ros2 run twist_limiter twist_limiter
```

Next, launch the gateway with the configuration from the previous section:

```console
source install/setup.bash
iox2 gateway ros2 --static-mapping mapping.toml --translator PlainStruct
```

Finally, publish velocity commands at 1 Hz that exceed the configured
maximum, and observe the limited result:

```console
source install/setup.bash
ros2 topic pub -r 1 /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 5.0}}"
```

```console
source install/setup.bash
ros2 topic echo /cmd_vel_limited
```

Every published message is received by the gateway over DDS and deserialized
directly into shared memory, where the limiter reads and writes plain
structs. On the way out, the gateway serializes the limited message back
into ROS 2:

```console
$ ros2 run twist_limiter twist_limiter
limited cmd_vel (sequence 1)
limited cmd_vel (sequence 2)
limited cmd_vel (sequence 3)
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
---
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
