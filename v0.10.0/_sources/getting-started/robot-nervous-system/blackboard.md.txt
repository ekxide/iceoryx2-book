# Blackboard

Now that our robot Larry can reliably drive from `A` to `B`, we want to tweak
the hardware and increase the update rate of our cyclic sensor data. For
example, we might decide that only sending out the ultrasonic sensor distance
every 100 ms might no longer be sufficient. Similarly, a user might want
to have the capability to configure the battery threshold at which Larry raises
a low-battery alarm.

As systems grow more complex, they often accumulate thousands of individual
configuration settings such as these. Some are shared between participants,
others are participant-specific. With hundreds of participants in play,
managing the configuration efficiently becomes critical.

A naïve approach would be to use publish-subscribe: where one participant
manages the configuration and publishes updates to all subscribing
participants, but this can quickly turn into a memory bottleneck. Consider a
1 MB configuration shared with 1000 participants. Since `iceoryx2` always
pre-allocates memory for the worst case, it must reserve 1001 MB of memory -
one copy for the publisher and one potential copy for each subscriber. Apart
from the memory waste, there is also the risk of configuration inconsistencies.

The solution is the blackboard messaging pattern: a key-value repository in
shared memory. Each participant can access exactly the entries it needs, such
as the ultrasonic sensor’s update rate. Any shared-memory-compatible and
trivially-copyable type, can be stored in the blackboard.

Let’s implement an example where a user app can configure the sensor update
rate. A higher rate means Larry reacts to obstacles faster and can drive faster.

## Writer

As always, we start by creating a node:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/rust/src/bin/writer.rs
:language: rust
:start-after: snippet:start node
:end-before: snippet:end node
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/python/writer.py
:language: python
:start-after: snippet:start node
:end-before: snippet:end node
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/cxx/src/writer.cpp
:language: c++
:start-after: snippet:start node
:end-before: snippet:end node
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/c/src/writer.c
:language: c
:start-after: snippet:start node
:end-before: snippet:end node
:dedent:
```
````

With the blackboard pattern, all key-value pairs must be defined when the
service is created. Unlike publish-subscribe, you cannot announce a key without
providing a value. That means exactly one participant is responsible for
creating the service and initializing default values. All other participants
may open the service later.

We now create a blackboard with two settings:

* the update rate in milliseconds, and
* the battery threshold for triggering the low-battery alarm.

For keys we use the `StaticString` type from the `iceoryx2` base library.
Currently, only Rust and C++ can share the `StaticString` in-memory. If the
blackboard is also to be used in C or Python, integers can be used instead.

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/rust/src/bin/writer.rs
:language: rust
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/python/writer.py
:language: python
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/cxx/src/writer.cpp
:language: c++
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/c/src/writer.c
:language: c
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```
````

Now we create a writer port to update values:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/rust/src/bin/writer.rs
:language: rust
:start-after: snippet:start writer
:end-before: snippet:end writer
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/python/writer.py
:language: python
:start-after: snippet:start writer
:end-before: snippet:end writer
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/cxx/src/writer.cpp
:language: c++
:start-after: snippet:start writer
:end-before: snippet:end writer
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/c/src/writer.c
:language: c
:start-after: snippet:start writer
:end-before: snippet:end writer
:dedent:
```
````

To update an entry, we first obtain a handle for it. The handle is type-checked,
so if you provide the wrong type, you’ll get an error instead of silently
corrupting the data.

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/rust/src/bin/writer.rs
:language: rust
:start-after: snippet:start entries
:end-before: snippet:end entries
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/python/writer.py
:language: python
:start-after: snippet:start entries
:end-before: snippet:end entries
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/cxx/src/writer.cpp
:language: c++
:start-after: snippet:start entries
:end-before: snippet:end entries
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/c/src/writer.c
:language: c
:start-after: snippet:start entries
:end-before: snippet:end entries
:dedent:
```
````

In the event loop, we periodically check for user input and update entries as
needed:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/rust/src/bin/writer.rs
:language: rust
:start-after: snippet:start update-loop
:end-before: snippet:end update-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/python/writer.py
:language: python
:start-after: snippet:start update-loop
:end-before: snippet:end update-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/cxx/src/writer.cpp
:language: c++
:start-after: snippet:start update-loop
:end-before: snippet:end update-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/c/src/writer.c
:language: c
:start-after: snippet:start update-loop
:end-before: snippet:end update-loop
:dedent:
```
````

## Reader

On the subscriber side, we reuse the node setup from the publish-subscribe
example and skip to opening the blackboard service:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/rust/src/bin/reader.rs
:language: rust
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/python/reader.py
:language: python
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/cxx/src/reader.cpp
:language: c++
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/c/src/reader.c
:language: c
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```
````

And create a reader port:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/rust/src/bin/reader.rs
:language: rust
:start-after: snippet:start reader
:end-before: snippet:end reader
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/python/reader.py
:language: python
:start-after: snippet:start reader
:end-before: snippet:end reader
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/cxx/src/reader.cpp
:language: c++
:start-after: snippet:start reader
:end-before: snippet:end reader
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/c/src/reader.c
:language: c
:start-after: snippet:start reader
:end-before: snippet:end reader
:dedent:
```
````

We’re interested in the `ultra_sonic_sensor_update_rate_in_ms`. We obtain a
handle that always points to the latest value:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/rust/src/bin/reader.rs
:language: rust
:start-after: snippet:start handle
:end-before: snippet:end handle
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/python/reader.py
:language: python
:start-after: snippet:start handle
:end-before: snippet:end handle
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/cxx/src/reader.cpp
:language: c++
:start-after: snippet:start handle
:end-before: snippet:end handle
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/c/src/reader.c
:language: c
:start-after: snippet:start handle
:end-before: snippet:end handle
:dedent:
```
````

The sensor loop looks almost identical to our original publisher code, except
that the update interval now comes from the global configuration:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/rust/src/bin/reader.rs
:language: rust
:start-after: snippet:start sensor-loop
:end-before: snippet:end sensor-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/python/reader.py
:language: python
:start-after: snippet:start sensor-loop
:end-before: snippet:end sensor-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/cxx/src/reader.cpp
:language: c++
:start-after: snippet:start sensor-loop
:end-before: snippet:end sensor-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/blackboard/c/src/reader.c
:language: c
:start-after: snippet:start sensor-loop
:end-before: snippet:end sensor-loop
:dedent:
```
````

## Related Examples

<!-- markdownlint-disable-next-line MD033 -->
<br/>

````{grid} 1 1 2 2

```{grid-item}
### {octicon}`code` Rust

{octicon}`mark-github` [Minimal Creator/Writer](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/blackboard/creator.rs)  
{octicon}`mark-github` [Minimal Opener/Reader](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/blackboard/opener.rs)
```

```{grid-item}
**{octicon}`code` Python**

{octicon}`mark-github` [Minimal Creator/Writer](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/blackboard/creator.py)  
{octicon}`mark-github` [Minimal Opener/Reader](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/blackboard/opener.py)
```

```{grid-item}
**{octicon}`code` C++**

{octicon}`mark-github` [Minimal Creator/Writer](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/blackboard/src/creator.cpp)  
{octicon}`mark-github` [Minimal Opener/Reader](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/blackboard/src/opener.cpp)
```

```{grid-item}
**{octicon}`code` C**

{octicon}`mark-github` [Minimal Creator/Writer](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/blackboard/src/creator.c)  
{octicon}`mark-github` [Minimal Opener/Reader](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/blackboard/src/opener.c)
```


````
