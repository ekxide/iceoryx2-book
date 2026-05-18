# Gateways and Tunnels

Gateways and Tunnels extend the communication of `iceoryx2` across host
boundaries by leveraging popular network communication middlewares and
libraries. They can be run in separate processes, thereby isolating network
communication from safety-critical code.

```{figure} /images/gateways-and-tunnels.svg
:alt: gateways and tunnels
:align: center
:name: fig-gateways-and-tunnels

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

```{grid-item-card} Understand the Communication Model
:link: /fundamentals/communication-model
:link-type: doc
:shadow: none

Get familiar with the components involved with establishing communication.
```

```{grid-item-card} Understand Shared Memory
:link: /fundamentals/shared-memory
:link-type: doc
:shadow: none

Build an intuition for what shared memory communication entails.
```

```{grid-item-card} Explore the Tunnel Implementation
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/iceoryx2-tunnel
:link-type: url
:shadow: none

Analyse the tunnel implementation and the available communication mechanisms.
```

````
