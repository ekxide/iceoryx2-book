# Layered Architecture

`iceoryx2` is organized into a layered architecture, which enables a
clear separation of concerns and modular design. Each layer builds upon
components provided by lower layers.

```{figure} /images/layered-architecture.svg
:alt: layered architecture of iceoryx2
:align: center
:name: fig-layered-architecture

Software Layers in the Architecture of `iceoryx2`
```

## User API (`iceoryx2`)

Components that applications use to interact with the `iceoryx2`
system.

## Concept Abstraction Layer (`iceoryx2-cal`)

Implementation of the domain-specific functionality and components of shared
memory communication. Each concept provides multiple implementations which can
be combined to make up an `iceoryx2` deployment.

## Building Blocks (`iceoryx2-bb`)

Reusable, optimized components that handle common computational
logic and data management patterns which are independent from the domain of
`iceoryx2` and shared-memory communication.

Analogous to a standard library distilled for `iceoryx2` and suitable for
certification.

## Platform Abstraction Layer (`iceoryx2-pal`)

Abstracts the functionality that `iceoryx2` requires from the platform it is
deployed to. Currently only contains a `POSIX` flavour, which is
implemented for all supported platforms.

Additional platforms can be added by implementing the abstraction using the
functionality of the target platform.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Understand Shared Memory
:link: /fundamentals/shared-memory
:link-type: doc
:shadow: none

Build an intuition for what shared memory communication entails.
```

```{grid-item-card} Understand the Communication Model
:link: /fundamentals/communication-model
:link-type: doc
:shadow: none

Get familiar with the components used to set up communication.
```

```{grid-item-card} Explore the Code
:link: https://github.com/eclipse-iceoryx/iceoryx2
:link-type: url
:shadow: none

See how the architecture is realized in the code in the repository.
```

````
