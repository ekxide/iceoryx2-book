# Build the ROS 2 gateway

Crates that integrate with ROS 2 are tightly coupled to the ROS 2 workspace
that they are built against as they link to the specific `rcl` library of the
workspace at build time.

A crate built against one workspace is likely not to work with a different
workspace.

Thus, it is recommended to build the ROS 2 integration crates from source for
the specific ROS 2 workspace you are using.

## Building the gateway

First clone the `iceoryx2` source:

```console
git clone git@github.com:eclipse-iceoryx/iceoryx2.git
cd iceoryx2
```

Then source your ROS 2 workspace before building the ROS 2 gateway:

```console
source /opt/ros/<distro>/setup.bash   # e.g. jazzy, humble, etc.
                                      # or source /path/to/install/setup.bash

cargo build --manifest-path integrations/ros2/Cargo.toml --bin iox2-link-gateway-ros2
```

```{important}
At runtime, the gateway loads the typesupport libraries of every message type
it propagates. The distribution provides them for its own message packages.
For custom message packages, source the workspace they are built in before
running the gateway.
```

This builds the ROS 2 gateway CLI along with the crates it consists of. To
verify the build was successful, try running the gateway CLI:

```console
cargo run --manifest-path integrations/ros2/Cargo.toml --bin iox2-link-gateway-ros2 -- --help
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
