# How Does It Fit?

## Different Approaches Solve Different Problems

There are many ways to exchange data between software components. The right choice depends on whether communication happens locally or across a network, which communication semantics are required, and how much of the surrounding architecture the communication technology should define.

Common approaches range from basic operating-system mechanisms to network communication protocols, feature-rich communication middleware, and larger software frameworks.

They solve different problems and come with different trade-offs.

## Operating-System Mechanisms

Operating systems provide inter-process communication (IPC) mechanisms such as sockets, pipes, or message queues that give applications a high degree of control over communication.

Building directly on these mechanisms, however, often means writing repeated and error-prone boilerplate code or developing and maintaining additional abstraction layers.

This can be a good fit for simple systems, but effort and complexity grow as the number of components and communication requirements increase. Adding shared memory later as a performance optimization can increase this complexity further.

## Network Communication Protocols

Network protocols are designed to exchange data between machines and provide features such as addressing, connection management, serialization, and interoperability across networks.

These properties are valuable for distributed systems. For high-volume communication between processes on the same machine, however, moving every payload through the network stack introduces overhead that is not required for local data exchange.

As message sizes or message rates increase, this overhead can lead to higher CPU utilization, increasing latency, and less predictable timing.

## Communication Middleware

Communication middleware can provide standardized APIs, service discovery, type systems, quality-of-service settings, and network communication as part of one integrated solution.

This can be a good choice when these concepts match the architecture of the system. Many such solutions also provide shared-memory transports to improve local IPC performance.

The trade-off is that the architecture can become coupled to that middleware's data model, communication semantics, network protocol, and execution concepts.

## Larger Frameworks

Larger frameworks go beyond communication and may also define application lifecycle, execution, diagnostics, tooling, and other parts of the software architecture.

This can significantly accelerate development when the framework matches the intended system.

At the same time, adopting a framework means adopting more architectural decisions with it and usually creates stronger coupling to a specific ecosystem.

## A Flexible Foundation for Your Architecture

`iceoryx2` can be used in different architectural roles depending on the needs of the system.

It can be integrated into existing communication stacks or frameworks as the zero-copy data plane, for example to optimize communication between processes without replacing the higher-level concepts already in use.

It can also serve as the foundation for custom communication architectures. The IDLs, network protocols, and other technologies that best fit the use case can be selected independently, while `iceoryx2` handles the architecturally important task of efficient and deterministic local data movement.

```{inline-svg} /images/iceoryx2-architecture-layers.svg
:alt: iceoryx2 in different software architectures

iceoryx2 as a Flexible Zero-Copy Data Plane
```

`iceoryx2` itself provides flexible mechanisms and extension points for combining the local data plane with external data models, IDLs, type systems, and network protocols through gateways and tunnels.

This allows `iceoryx2` to optimize an existing stack or serve as a building block for a custom architecture without imposing a framework or ecosystem.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Fundamentals: Gateways and Tunnels
:link: /fundamentals/gateways-and-tunnels
:link-type: doc
:shadow: none

Learn how the local `iceoryx2` data plane can connect to other communication technologies and networks.
```

```{grid-item-card} Fundamentals: Communication Model
:link: /fundamentals/communication-model
:link-type: doc
:shadow: none

Learn more about the communication model behind `iceoryx2`.
```

```{grid-item-card} Fundamentals: Layered Architecture
:link: /fundamentals/layered-architecture
:link-type: doc
:shadow: none

Learn more about the layers of `iceoryx2`.
```

````
