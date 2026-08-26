# Why Does It Matter?

## Data Movement Becomes Part of the Workload

Modern data-intensive systems are built from many software components that continuously exchange data.

A camera frame may pass through perception, recording, and visualization. A robot may combine data from cameras, LiDAR, IMU, and joint sensors. Each processing step consumes data, produces new data, and passes it on.

As these systems grow, the cost of moving data between components can become a significant part of the overall workload.

This is especially relevant for physical AI and other embedded systems, where large data volumes have to be processed with limited CPU and within strict latency bounds.

## Large Payloads Make Copies Expensive

On POSIX systems, processes operate in separate virtual address spaces. Moving data from one process to another therefore usually requires help from the operating system or a communication middleware.

Depending on the communication mechanism, a single transfer can involve serialization and deserialization, multiple data copies, system calls, user-kernel transitions, and context switches.

```{inline-svg} /images/sdv-posix-comm.svg
:alt: POSIX IPC

Typical Inter-Process Communication on POSIX
```

For small messages, this overhead can be negligible. For large sensor payloads, it scales with the amount of data being transferred. 

For example, consider an automated-driving system with several high-resolution cameras producing several GB/s of sensor data. If this data is copied multiple times while moving through the system, CPU time and memory bandwidth are spent on transporting data instead of processing it.

The result is:

* higher CPU utilization
* increasing communication latency
* additional memory-bandwidth pressure
* less compute available for the actual workload

## Message Rate Matters Too

Payload size is only one dimension of communication overhead.

Modern systems can also exchange thousands of smaller messages every second. If handling each message requires operating-system calls, context switches, involvement of middleware threads, or per-message callbacks, the accumulated scheduling overhead can become significant.

This is particularly problematic for real-time systems. Background activity and additional context switches not only consume CPU time, but can also introduce latency variation and make execution harder to predict.

A communication mechanism therefore needs to scale in two directions:

* with the **size of the data** being exchanged
* with the **number of interactions** between software components

## Data Movement Is an Architectural Concern

When communication overhead is small, the mechanism used to move data between software components may appear to be an implementation detail.

In data-intensive systems, it is not.

The way data is moved affects CPU utilization, latency, determinism, and how well the system scales as workloads grow. This makes the data movement layer an architectural decision rather than just a transport detail.

`iceoryx2` addresses this problem with a zero-copy data plane designed for efficient and deterministic data movement.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Fundamentals: Shared Memory
:link: /fundamentals/shared-memory
:link-type: doc
:shadow: none

Understand the mechanism that eliminates payload copies.
```

```{grid-item-card} Fundamentals: Execution Control
:link: /fundamentals/execution-control
:link-type: doc
:shadow: none

Understand how `iceoryx2` avoids forcing communication and execution together.
```

```{grid-item-card} iceoryx2 Performance Benchmarks
:link: https://github.com/eclipse-iceoryx/iceoryx2#performance
:link-type: url
:shadow: none

See the resulting latency characteristics.
```

````
