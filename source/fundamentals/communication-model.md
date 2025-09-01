# Communication Model

## Nodes

```{figure} /images/nodes-lifecycle-management.svg
:alt: nodes lifecycle management
:align: center

Lifecycle Management via Nodes
```

Nodes are the entry-point for participants to interact with the `iceoryx2`
system. They provide the mechanisms for creating services and their ports, and
managing their lifetimes.

Nodes are also responsible for detecting and handling failures in
other nodes. This peer-to-peer monitoring ensures that system resources don't
become permanently lost due to failed participants.

Generally, each participant is represented by single node.

## Services

```{figure} /images/service-orientation.svg
:alt: service oriented nature of iceoryx2
:align: center
:name: fig-service-oriented-communication-model

Service-oriented Communication Graph
```

The communication graph between participants in an `iceoryx2` system is
determined using services.

A service is a construct defined by a unique service name and a
[messaging pattern](/fundamentals/messaging-patterns/index), and
[quality-of-service (QoS) properties](https://docs.rs/iceoryx2/0.6.1/iceoryx2/#quality-of-services).
Participants may create new services that do not already exist in the
system or open existing services created by other participants:

* When creating a service, a participant defines the terms for
communication by specifying the messaging pattern and quality-of-service
properties
* When operning a service, a participant may specify its own quality-of-service
properties, however they must be compatible in order for communication to occur

This service-centric approach provides several advantages: it decouples
participants from direct knowledge of each other, enables dynamic discovery of
communication partners, and allows the system to enforce consistent
communication contracts across all participants.

## Ports

Ports are the interfaces for interacting with services in an `iceoryx2` system
and represent specific roles in the communication over a service.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Understand Execution Control
:link: /fundamentals/execution-control
:link-type: doc
:shadow: none

Understand how `Event` services fit into the bigger picture of execution control
with `iceoryx2`.
```

```{grid-item-card} Understand Gateways and Tunnels
:link: /fundamentals/gateways-and-tunnels
:link-type: doc
:shadow: none

Understand how to extend `iceoryx2` across hosts.
```

```{grid-item-card} Explore Quality-of-Service Properties
:link: https://docs.rs/iceoryx2/latest/iceoryx2/#quality-of-services
:link-type: url
:shadow: none

View the currently supported quality-of-service properties.
```

````
