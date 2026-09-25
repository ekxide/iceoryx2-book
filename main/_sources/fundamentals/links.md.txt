# Links

A link extends `iceoryx2` communication beyond its shared memory domain, for
example to other hosts on a network, to virtual machines under a hypervisor, or
to co-processors with memory of their own. Links can run in separate processes,
isolating non-deterministic communication, such as networking, from
safety-critical code.

```{inline-svg} /images/links.svg
:name: fig-links
:alt: links from an iceoryx2 system to another middleware and another iceoryx2 system

A Gateway and a Tunnel Extending an iceoryx2 System
```

## Link Backends

Each link combines a common core, which discovers local services and moves their
data, with a generic backend, which knows the opposing side and how to reach it.
Services offered locally are propagated through the backend, and services found
through it appear as local services, so applications use them like any other.
Two kinds of backend are provided out-of-the-box, which can be extended to new
mechanisms and middlewares through carriers and adapters. If these are not
sufficient, a new backend kind can be implemented.

### Gateways

Gateways connect `iceoryx2` to another middleware through an adapter. Besides
the adapter, a gateway is configured with a mapping, which decides which
services correspond to which of the middleware's endpoints, and a translator,
which converts data between their formats. This lets applications that
do not use `iceoryx2` participate in the communication.

An adapter integrates one middleware, listing its endpoints and exchanging
messages with them. Another middleware is supported by implementing an adapter
for it. An adapter for ROS 2 is provided.

### Tunnels

Tunnels connect `iceoryx2` systems through a carrier, such as a network protocol
or a hypervisor channel. Samples cross as-is, in the exact form they have in
shared memory. By avoiding data transformation, tunnels minimize processing
overhead.

A carrier integrates one mechanism, announcing services to its peers and
carrying their bytes. Another mechanism is supported by implementing a carrier
for it. A carrier over Zenoh is provided.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Explore the Link Implementation
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/iceoryx2-link
:link-type: url
:shadow: none

Browse the link, its backends and their contracts.
```

```{grid-item-card} Explore the Zenoh Tunnel
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/integrations/zenoh/link-carrier
:link-type: url
:shadow: none

See a carrier tunneling `iceoryx2` over Zenoh.
```

```{grid-item-card} Explore the ROS 2 Gateway
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/integrations/ros2/link-adapter
:link-type: url
:shadow: none

See an adapter integrating `iceoryx2` with ROS 2.
```

````
