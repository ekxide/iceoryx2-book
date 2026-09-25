# Links

Gateways and Tunnels extend the communication of `iceoryx2` across host
boundaries by leveraging popular network communication middlewares and
libraries. They can be run in separate processes, thereby isolating network
communication from safety-critical code.

```{inline-svg} /images/links.svg
:name: fig-links
:alt: gateways and tunnels

Host-to-host Communication
```

## Gateways

Gateways translate `iceoryx2` payloads into data formats used by different
network communication middlewares and libraries. In addition to extending the
communication beyond a single host, this translation enables applications that
are not using `iceoryx2` to participate in the communication.

## Tunnels

Tunnels propagate raw `iceoryx2` payloads as-is between hosts running
`iceoryx2`. By avoiding data transformation, tunnels minimize processing
overhead and maintain the native `iceoryx2` communication semantics
across distributed systems.

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
