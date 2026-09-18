# Where Is It Going?

The long-term vision for `iceoryx2` is to provide a zero-copy data plane that
goes beyond efficient communication between processes on a single operating
system. The goal is to make the same data plane available across the
heterogeneous hardware and software architectures found in modern embedded
systems.

```{inline-svg} /images/grand-vision.svg
:alt: iceoryx2 vision

The iceoryx2 zero-copy data plane across heterogeneous systems
```

## Additional Platforms

Modern systems combine different operating systems, programming languages,
and processor architectures.

`iceoryx2` already supports a broad range of them, but the goal is to extend
this further:

* more operating systems
* more programming-language bindings
* more hardware architectures

Applications should be able to use the same communication concepts across
these environments instead of introducing a different communication
technology for every platform.

## Zero-Copy Beyond POSIX Processes

Shared memory provides efficient zero-copy communication between processes
that can access the same memory.

Modern embedded architectures, however, increasingly consist of multiple
memory domains. Applications may run on heterogeneous CPU cores, inside
separate hypervisor partitions, or use accelerators such as GPUs.

The goal is to extend the data plane across these boundaries, including:

* communication between application and real-time cores
* communication across hypervisor partitions
* direct data exchange between CPUs and GPUs

The underlying memory mechanism may change, and in some cases a copy may be
unavoidable, but applications should continue to use the same `iceoryx2`
communication model.

## A Data Plane Easy to Extend

`iceoryx2` is intended to remain a data plane rather than grow into a
framework that defines the complete software stack.

The goal is therefore to make it increasingly easy to combine `iceoryx2`
with the technologies that surround the data movement layer.

This includes integration with common data models and IDLs as well as tunnels
and gateways for different network protocols. Applications can select the
technologies that best fit their architecture while using `iceoryx2` as the
common zero-copy foundation underneath.

The goal is to expand the capabilities and reach of the data plane without
giving up its lean architecture, predictable behavior, and flexibility.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Fundamentals: Gateways and Tunnels
:link: /fundamentals/gateways-and-tunnels
:link-type: doc
:shadow: none

Learn how `iceoryx2` can connect the local data plane to networks and other
communication technologies.
```

```{grid-item-card} Fundamentals: Cross-Language Support
:link: /fundamentals/cross-language-support
:link-type: doc
:shadow: none

Learn how applications written in different languages communicate through
`iceoryx2`.
```

```{grid-item-card} Get Started with iceoryx2
:link: /getting-started/quickstart
:link-type: doc
:shadow: none

Build and run your first `iceoryx2` application.
```

````
