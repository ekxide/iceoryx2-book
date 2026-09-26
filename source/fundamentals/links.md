# Links

A link extends `iceoryx2` communication beyond its shared memory domain. It can
be used to connect to other middlewares, or to other `iceoryx2` systems outside
its shared memory domain, such as those on other hosts, on other processors on
the same board, or in virtual machines without shared memory between them.

A link discovers what is offered at both ends of the connection and makes it
available at the other end. Applications use services made available by a link
like any local service, and communicate across it without changes to their
code.

Links can run in separate processes, isolating non-deterministic
communication, such as networking, from safety-critical code.

```{inline-svg} /images/links.svg
:name: fig-links
:alt: links from an iceoryx2 system to another middleware and another iceoryx2 system

A Gateway and a Tunnel Extending an iceoryx2 System
```

## Link Backends

Each link combines a common core with a generic backend:

* The core handles the local `iceoryx2` system. It discovers the services
  offered locally and moves their data in and out of shared memory.
* The backend handles the opposing side. It discovers what is offered there and
  moves data across the boundary.

Two kinds of backend are provided out-of-the-box, gateways and tunnels.
Gateways are extended to new middlewares through adapters, and tunnels to new
mechanisms through carriers. If neither kind fits, a new backend kind can be
implemented.

### Gateways

Gateways connect `iceoryx2` to another middleware through an adapter. Besides
the adapter, a gateway is configured with a mapping, which decides which
services correspond to which of the middleware's endpoints, and a translator,
which converts data between their formats. This lets applications that
do not use `iceoryx2` participate in the communication.

An adapter integrates a single middleware. It lists the middleware's endpoints
and exchanges messages with them. An adapter for ROS 2 is provided, and further
middlewares can be supported by implementing additional adapters.

### Tunnels

Tunnels connect `iceoryx2` systems through a carrier, such as a network protocol
or a hypervisor channel. Samples cross as-is, in the exact form they have in
shared memory. By avoiding data transformation, tunnels minimize processing
overhead.

A carrier integrates a single communication mechanism. It announces services
to its peers and carries their bytes. A carrier over Zenoh is provided, and
further mechanisms can be supported by implementing additional carriers.

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
