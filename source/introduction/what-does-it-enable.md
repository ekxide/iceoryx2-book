# What Does It Enable?

## Low Latency and More Compute for the Workload

By avoiding payload copies in the local data path, `iceoryx2` minimizes the CPU time and memory bandwidth spent on communication. At the same time, message-delivery latency remains low and independent of payload size.

This results in sub-microsecond message-delivery latency, even for large payloads, while leaving more of the available compute resources for the algorithms and application logic that actually process the data.

```{inline-svg} /images/latency.svg
:alt: iceoryx2 message-delivery latency

Message-delivery latency with increasing payload size
```

For data-intensive systems, this allows larger workloads to run on the same hardware and helps scale applications from powerful development machines to more resource-constrained embedded targets.

## Predictable Execution

Fast communication alone is not enough for real-time systems. Communication also needs to be predictable and avoid unexpected background activity that interferes with application execution.

`iceoryx2` has no background threads, no system calls in the data-delivery path, and no central broker. Message delivery therefore does not depend on hidden middleware activity or additional scheduling decisions.

Together with the separation of data flow and control flow, this gives applications explicit control over when threads wake up, when data is consumed, and where execution takes place.

The result is stable low latency with minimal jitter and communication behavior that is easier to reason about under real-time constraints.

## Move Toward Mission-Critical Deployment

`iceoryx2` is designed for demanding embedded environments where predictable resource usage and explicit control over system behavior matter.

It supports `no_std` environments and avoids heap allocations during runtime. Communication resources are bounded and configurable, while the decentralized architecture avoids a central component that all communication depends on.

These properties make `iceoryx2` a suitable communication foundation for systems with real-time, robustness, and resource constraints.

They do not make a complete system safe or certified by themselves. But starting with a data plane designed for these constraints helps avoid replacing a fundamental part of the architecture when moving from prototype to production.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Fundamentals: Execution Control
:link: /fundamentals/execution-control
:link-type: doc
:shadow: none

Learn how `iceoryx2` keeps execution under application control.
```

```{grid-item-card} iceoryx2 Performance Benchmarks
:link: https://github.com/eclipse-iceoryx/iceoryx2#performance
:link-type: url
:shadow: none

Explore the message-delivery latency characteristics of `iceoryx2`.
```

```{grid-item-card} no_std Builds
:link: https://github.com/eclipse-iceoryx/iceoryx2/blob/main/doc/development-setup/nostd-builds.md
:link-type: url
:shadow: none

Learn how `iceoryx2` can be built for `no_std` environments.
```

````
