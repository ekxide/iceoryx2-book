# Publish–Subscribe

Larry’s hardware and algorithms are already built. Now we want to send the
distance measured by his ultrasonic sensor to another participant that can slam
the brakes if an obstacle gets closer than X meters.

In `iceoryx2`, that’s a perfect job for the **publish-subscribe** pattern: one
participant publishes a stream of distances, another subscribes and reacts.

```{inline-svg} /images/robot-nervous-system-publish-subscribe-graph.svg
:name: fig-wiring-a-robot-brain-publish-subscribe
:alt: Communication graph of Ultrasonic Sensor and Emergency Brake

The communication graph of the Ultrasonic Sensor and Emergency Brake
participants
```

## Publisher

Everything in `iceoryx2` starts with a node. A node represents a communication
point (like a process or thread) and acts as a factory for services. To keep
things sane when debugging later, we’ll give this node a name:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/rust/src/bin/publisher.rs
:language: rust
:start-after: snippet:start node
:end-before: snippet:end node
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/python/publisher.py
:language: python
:start-after: snippet:start node
:end-before: snippet:end node
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/cxx/src/publisher.cpp
:language: c++
:start-after: snippet:start node
:end-before: snippet:end node
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/c/src/publisher.c
:language: c
:start-after: snippet:start node
:end-before: snippet:end node
:dedent:
```
````

Now we can create a service called `"distance_to_obstacle"`, using a struct
`Distance` as the payload type. Payloads must be shared-memory compatible,
meaning:

* self-contained
* no internal pointers
* identical memory representation

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/rust/src/distance.rs
:language: rust
:start-after: snippet:start payload
:end-before: snippet:end payload
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/python/distance.py
:language: python
:start-after: snippet:start payload
:end-before: snippet:end payload
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/cxx/src/distance.hpp
:language: c++
:start-after: snippet:start payload
:end-before: snippet:end payload
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/c/src/distance.h
:language: c
:start-after: snippet:start payload
:end-before: snippet:end payload
:dedent:
```
````

With the payload defined, we can set up the service:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/rust/src/bin/publisher.rs
:language: rust
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/python/publisher.py
:language: python
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/cxx/src/publisher.cpp
:language: c++
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/c/src/publisher.c
:language: c
:start-after: snippet:start service
:end-before: snippet:end service
:dedent:
```
````

Now we create our publisher:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/rust/src/bin/publisher.rs
:language: rust
:start-after: snippet:start publisher
:end-before: snippet:end publisher
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/python/publisher.py
:language: python
:start-after: snippet:start publisher
:end-before: snippet:end publisher
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/cxx/src/publisher.cpp
:language: c++
:start-after: snippet:start publisher
:end-before: snippet:end publisher
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/c/src/publisher.c
:language: c
:start-after: snippet:start publisher
:end-before: snippet:end publisher
:dedent:
```
````

Larry isn’t exactly Formula 1 material, so publishing every 100 ms is plenty. We
set up a loop that waits 100 ms, reads the sensor, and sends the data. To fully
benefit from zero-copy, we don’t allocate or clone data; instead we loan an
uninitialized sample from the publisher’s memory pool, fill it, and then send
it:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/rust/src/bin/publisher.rs
:language: rust
:start-after: snippet:start publish-loop
:end-before: snippet:end publish-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/python/publisher.py
:language: python
:start-after: snippet:start publish-loop
:end-before: snippet:end publish-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/cxx/src/publisher.cpp
:language: c++
:start-after: snippet:start publish-loop
:end-before: snippet:end publish-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/c/src/publisher.c
:language: c
:start-after: snippet:start publish-loop
:end-before: snippet:end publish-loop
:dedent:
```
````

Whenever we acquire an uninitialized sample, we must write the payload to it and
convert it into an initialized sample. This ensures we don’t accidentally ship
uninitialized garbage across participants. Once the payload is written, the
sample is safe to send.

## Subscriber

The subscriber setup starts the same: create a node, open the
`"distance_to_obstacle"` service, and specify the payload type.

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/rust/src/bin/subscriber.rs
:language: rust
:start-after: snippet:start node-and-service
:end-before: snippet:end node-and-service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/python/subscriber.py
:language: python
:start-after: snippet:start node-and-service
:end-before: snippet:end node-and-service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/cxx/src/subscriber.cpp
:language: c++
:start-after: snippet:start node-and-service
:end-before: snippet:end node-and-service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/c/src/subscriber.c
:language: c
:start-after: snippet:start node-and-service
:end-before: snippet:end node-and-service
:dedent:
```
````

Now we create the subscriber side:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/rust/src/bin/subscriber.rs
:language: rust
:start-after: snippet:start subscriber
:end-before: snippet:end subscriber
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/python/subscriber.py
:language: python
:start-after: snippet:start subscriber
:end-before: snippet:end subscriber
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/cxx/src/subscriber.cpp
:language: c++
:start-after: snippet:start subscriber
:end-before: snippet:end subscriber
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/c/src/subscriber.c
:language: c
:start-after: snippet:start subscriber
:end-before: snippet:end subscriber
:dedent:
```
````

Since the publisher sends updates every 100 ms, we loop at the same pace and
check for new data. If we receive something, we print it:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/rust/src/bin/subscriber.rs
:language: rust
:start-after: snippet:start receive-loop
:end-before: snippet:end receive-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/python/subscriber.py
:language: python
:start-after: snippet:start receive-loop
:end-before: snippet:end receive-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/cxx/src/subscriber.cpp
:language: c++
:start-after: snippet:start receive-loop
:end-before: snippet:end receive-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/publish-subscribe/c/src/subscriber.c
:language: c
:start-after: snippet:start receive-loop
:end-before: snippet:end receive-loop
:dedent:
```
````

From here, reacting is easy: if the distance falls below a threshold, Larry can
hit the emergency brake before smashing into a wall.

## Related Examples

<!-- markdownlint-disable-next-line MD033 -->
<br/>

````{grid} 1 1 2 2
:gutter: 3

```{grid-item}
**{octicon}`code` Rust**

{octicon}`mark-github` [Minimal Publisher](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/publish_subscribe/publisher.rs)  
{octicon}`mark-github` [Minimal Subscriber](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/publish_subscribe/subscriber.rs)
```

```{grid-item}
**{octicon}`code` Python**

{octicon}`mark-github` [Minimal Publisher](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/publish_subscribe/publisher.py)  
{octicon}`mark-github` [Minimal Subscriber](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/publish_subscribe/subscriber.py)
```

```{grid-item}
**{octicon}`code` C++**

{octicon}`mark-github` [Minimal Publisher](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/publish_subscribe/src/publisher.cpp)  
{octicon}`mark-github` [Minimal Subscriber](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/publish_subscribe/src/subscriber.cpp)
```

```{grid-item}
**{octicon}`code` C**

{octicon}`mark-github` [Minimal Publisher](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/publish_subscribe/src/publisher.c)  
{octicon}`mark-github` [Minimal Subscriber](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/publish_subscribe/src/subscriber.c)
```

````
