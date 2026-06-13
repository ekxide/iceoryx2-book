# Event-Driven Communication

Before jumping into the example, we need to clarify the difference between
**data flow** and **control flow**.

In programming, control flow is about _when_ a function executes, while data
flow is about _what inputs_ it receives. In `iceoryx2` these two concepts are
separated on purpose, unlike in many network protocols where receiving data
automatically involves a syscall that wakes up the participant.

Messaging patterns like publish-subscribe and request-response define the data
flow. The event messaging pattern handles control flow. Why split them? Because
you don’t always want to pass data when triggering execution. Sometimes you just
need to wake up a function with no additional input. Other times you need
several inputs, and you don’t want to be interrupted until all of them have
arrived.
Mixing data and control flow forces you into hacks like sending empty messages
or waking up too early.

By keeping them separate, `iceoryx2` gives you explicit control over _when_ your
system reacts.

## Why It Matters

Take the emergency brake system of our robot Larry. It doesn’t need every single
distance sample from the ultrasonic sensor. If the next obstacle detected is on
another planet, we can ignore it. But when an obstacle is close, we suddenly
care. And not just about the raw number but also the trend over time. If the
object is receding, no action is needed; if it’s approaching, we may need to hit
the brakes.

Now consider failure cases. If the sensor participant dies and never sends
another update, Larry shouldn’t drive happily into oblivion. A safe fallback
(parking, for example) is required. Another factor to consider: the emergency
brake participant might start after the sensor participant.
In that case, it still needs the most recent three distance samples to compute
not just position, but relative speed and acceleration.

These scenarios show why we want independent streams for data and control
signals.

## Publisher Setup

We start by creating a node and defining two services with the same name:

* a publish-subscribe service for distance samples,
* an event service for control signals.

The subscriber needs the last three samples whenever it connects, so we
configure both the subscriber buffer and history size accordingly. The
subscriber also needs to hold on to/borrow three samples in parallel to compute
position, speed, and acceleration.

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/rust/src/bin/publisher.rs
:language: rust
:start-after: snippet:start pubsub-service
:end-before: snippet:end pubsub-service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/python/publisher.py
:language: python
:start-after: snippet:start pubsub-service
:end-before: snippet:end pubsub-service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/cxx/src/publisher.cpp
:language: c++
:start-after: snippet:start pubsub-service
:end-before: snippet:end pubsub-service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/c/src/publisher.c
:language: c
:start-after: snippet:start pubsub-service
:end-before: snippet:end pubsub-service
:dedent:
```
````

For the event service, we configure a special event that fires if the sensor
participant is identified as dead, allowing the emergency brake to switch to a
safe state.

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/rust/src/bin/publisher.rs
:language: rust
:start-after: snippet:start event-service
:end-before: snippet:end event-service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/python/publisher.py
:language: python
:start-after: snippet:start event-service
:end-before: snippet:end event-service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/cxx/src/publisher.cpp
:language: c++
:start-after: snippet:start event-service
:end-before: snippet:end event-service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/c/src/publisher.c
:language: c
:start-after: snippet:start event-service
:end-before: snippet:end event-service
:dedent:
```
````

Now we create a publisher for the distance samples and a notifier for control
events.

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/rust/src/bin/publisher.rs
:language: rust
:start-after: snippet:start ports
:end-before: snippet:end ports
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/python/publisher.py
:language: python
:start-after: snippet:start ports
:end-before: snippet:end ports
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/cxx/src/publisher.cpp
:language: c++
:start-after: snippet:start ports
:end-before: snippet:end ports
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/c/src/publisher.c
:language: c
:start-after: snippet:start ports
:end-before: snippet:end ports
:dedent:
```
````

The publishing loop: send the distance sample every 100 ms, and trigger an
event if it’s below the threshold.

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/rust/src/bin/publisher.rs
:language: rust
:start-after: snippet:start publish-loop
:end-before: snippet:end publish-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/python/publisher.py
:language: python
:start-after: snippet:start publish-loop
:end-before: snippet:end publish-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/cxx/src/publisher.cpp
:language: c++
:start-after: snippet:start publish-loop
:end-before: snippet:end publish-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/c/src/publisher.c
:language: c
:start-after: snippet:start publish-loop
:end-before: snippet:end publish-loop
:dedent:
```
````

## Subscriber Setup

On the other side, we again create both services and their ports:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/rust/src/bin/subscriber.rs
:language: rust
:start-after: snippet:start setup
:end-before: snippet:end setup
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/python/subscriber.py
:language: python
:start-after: snippet:start setup
:end-before: snippet:end setup
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/cxx/src/subscriber.cpp
:language: c++
:start-after: snippet:start setup
:end-before: snippet:end setup
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/c/src/subscriber.c
:language: c
:start-after: snippet:start setup
:end-before: snippet:end setup
:dedent:
```
````

Instead of polling every 100 ms, the subscriber just waits for events. When
woken up, it processes them and goes back to sleep.

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/rust/src/bin/subscriber.rs
:language: rust
:start-after: snippet:start wait-loop
:end-before: snippet:end wait-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/python/subscriber.py
:language: python
:start-after: snippet:start wait-loop
:end-before: snippet:end wait-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/cxx/src/subscriber.cpp
:language: c++
:start-after: snippet:start wait-loop
:end-before: snippet:end wait-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event-driven-communication/c/src/subscriber.c
:language: c
:start-after: snippet:start wait-loop
:end-before: snippet:end wait-loop
:dedent:
```
````

## Health Monitoring

The notifier’s “dead event” relies on health monitoring: `iceoryx2` offers
building blocks that detects when a participant dies. How this works in detail
is covered in a separate tutorial.

## Related Examples

<!-- markdownlint-disable-next-line MD033 -->
<br/>

````{grid} 1 1 2 2
:gutter: 3

```{grid-item}
**{octicon}`code` Rust**

{octicon}`mark-github` [Minimal Event-Driven Publisher](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/event_based_communication/publisher.rs)  
{octicon}`mark-github` [Minimal Event-Driven Subscriber](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/event_based_communication/subscriber.rs)
```

```{grid-item}
**{octicon}`code` Python**

Functionality available but needs an example.  
Want to contribute?  
{octicon}`mark-github` [Create Pull Request](https://github.com/eclipse-iceoryx/iceoryx2/pulls)
```

```{grid-item}
**{octicon}`code` C++**

{octicon}`mark-github` [Minimal Event-Driven Publisher](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/event_based_communication/src/publisher.cpp)  
{octicon}`mark-github` [Minimal Event-Driven Subscriber](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/event_based_communication/src/subscriber.cpp)
```

```{grid-item}
**{octicon}`code` C**

Functionality available but needs an example.  
Want to contribute?  
{octicon}`mark-github` 
[Create Pull Request](https://github.com/eclipse-iceoryx/iceoryx2/pulls)
```

````
