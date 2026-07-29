# Build ROS 2 integration crates

Crates that integrate with ROS 2 are tightly coupled to the ROS 2 workspace
that they are built against as they link to the specific `rcl` library of the
workspace at build time.

A crate built against one workspace is likely not to work with a different
workspace.

Thus, it is recommended to build the ROS 2 integration crates from source for
the specific ROS 2 workspace you are using.

## Building the crates

First clone the `iceoryx2` source:

```console
git clone git@github.com:eclipse-iceoryx/iceoryx2.git
```

Then source your ROS 2 workspace before building the ROS 2 integration crates:

```console
source /opt/ros/<distro>/setup.bash   # e.g. jazzy, humble, etc.
                                      # or source /path/to/install/setup.bash

cargo build --manifest-path integrations/ros2/Cargo.toml
```

```{important}
The crates may additionally need to dynamically load typesupport libraries for
your message types at runtime, so the same workspace must be sourced before
running any of the binaries.
```

## Building the examples

The examples are native `iceoryx2` applications packaged as ROS 2
`ament_cargo` packages. They are excluded from the integration workspace and
built with `colcon`, since they link against the Rust message crates that
`rosidl_generator_rs` generates into a ROS 2 install space.

In the following, `<distro>` is the compatible ROS 2 distribution
(e.g. humble, jazzy, etc).

### Prerequisites

Building an `ament_cargo` package requires the cargo-aware `colcon` build
types, along with `vcstool` to import the message sources:

```console
pip install colcon-cargo colcon-ros-cargo vcstool
cargo install cargo-ament-build
```

### Building

```{important}
Ensure to run `colcon` from the **colcon workspace** root
(`integrations/ros2/workspace/`) so that the build is properly configured to find
the generated Rust messages.
```

The distributions do not ship the generated Rust messages, so their sources are
imported into the workspace and generated first:

```console
cd integrations/ros2/workspace
source /opt/ros/<distro>/setup.bash

vcs import src < <distro>.repos
colcon build --packages-up-to std_msgs geometry_msgs rosidl_generator_rs
```

Then the example nodes themselves can be built:

```console
source install/setup.bash
colcon build --packages-select demo_nodes_iceoryx2
```

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} ROS 2 Integrations
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/integrations/ros2
:link-type: url
:shadow: none

Overview of the ROS 2 integrations and the containerized development
environment they are developed in.
```

```{grid-item-card} Example Nodes
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/integrations/ros2/workspace/src/demo_nodes
:link-type: url
:shadow: none

Reference examples of `iceoryx2` applications bridged to ROS 2 by the tunnel.
```

```{grid-item-card} Understand Gateways and Tunnels
:link: /fundamentals/gateways-and-tunnels
:link-type: doc
:shadow: none

Get familiar with how `iceoryx2` extends communication across host boundaries.
```

````
