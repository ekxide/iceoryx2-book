# Event

Larry now has his distance sensor sorted out, but he’s still clumsy. We’ll add
two more sensors:

* A **battery sensor** that warns the system health monitor when juice is
  running low.
* A **bump sensor** that tattles when Larry walks into a wall.

This is where the event messaging pattern comes in. Instead of streaming values,
a participant can send a one-off notification through a notifier port to another
participant that is sleeping on a listener port. The listener wakes up only when
something worth caring about happens.

In our case: low battery or wall collision. The health monitor reacts by
lighting up the battery LED or going into a parking position.

Before diving into code, let’s clear up two terms:

* **State**: the current facts in memory (e.g. the battery is low or normal).
* **Event**: the notification that state changed.

If we hammer the listener with “battery low” notifications, it will still only
see one event until it wakes up and resets the flag. This prevents events from
being lost, but also means they don’t queue up like publish-subscribe messages.
Order is undefined; you only know that at least once someone signaled a state
change.

## Notifier

As usual, everything begins with a node and a service, but this time we create
an event service:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/event/rust/src/bin/notifier.rs
:language: rust
:start-after: snippet:start node-and-service
:end-before: snippet:end node-and-service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/python/notifier.py
:language: python
:start-after: snippet:start node-and-service
:end-before: snippet:end node-and-service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/cxx/src/notifier.cpp
:language: c++
:start-after: snippet:start node-and-service
:end-before: snippet:end node-and-service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/c/src/notifier.c
:language: c
:start-after: snippet:start node-and-service
:end-before: snippet:end node-and-service
:dedent:
```
````

Now we create the notifier port, which sends notifications:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/event/rust/src/bin/notifier.rs
:language: rust
:start-after: snippet:start notifier
:end-before: snippet:end notifier
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/python/notifier.py
:language: python
:start-after: snippet:start notifier
:end-before: snippet:end notifier
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/cxx/src/notifier.cpp
:language: c++
:start-after: snippet:start notifier
:end-before: snippet:end notifier
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/c/src/notifier.c
:language: c
:start-after: snippet:start notifier
:end-before: snippet:end notifier
:dedent:
```
````

We want to notify about two distinct things: wall collisions and low battery.
So we define two event IDs:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/event/rust/src/bin/notifier.rs
:language: rust
:start-after: snippet:start event-ids
:end-before: snippet:end event-ids
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/python/notifier.py
:language: python
:start-after: snippet:start event-ids
:end-before: snippet:end event-ids
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/cxx/src/notifier.cpp
:language: c++
:start-after: snippet:start event-ids
:end-before: snippet:end event-ids
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/c/src/notifier.c
:language: c
:start-after: snippet:start event-ids
:end-before: snippet:end event-ids
:dedent:
```
````

Checking the sensors every second is good enough for health monitoring, so we
send notifications only when something changes:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/event/rust/src/bin/notifier.rs
:language: rust
:start-after: snippet:start notify-loop
:end-before: snippet:end notify-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/python/notifier.py
:language: python
:start-after: snippet:start notify-loop
:end-before: snippet:end notify-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/cxx/src/notifier.cpp
:language: c++
:start-after: snippet:start notify-loop
:end-before: snippet:end notify-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/c/src/notifier.c
:language: c
:start-after: snippet:start notify-loop
:end-before: snippet:end notify-loop
:dedent:
```
````

## Listener

On the other side, the listener is the participant waiting for the health events.
First, create the node and open the event service:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/event/rust/src/bin/listener.rs
:language: rust
:start-after: snippet:start node-and-service
:end-before: snippet:end node-and-service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/python/listener.py
:language: python
:start-after: snippet:start node-and-service
:end-before: snippet:end node-and-service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/cxx/src/listener.cpp
:language: c++
:start-after: snippet:start node-and-service
:end-before: snippet:end node-and-service
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/c/src/listener.c
:language: c
:start-after: snippet:start node-and-service
:end-before: snippet:end node-and-service
:dedent:
```
````

Now we create the listener port:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/event/rust/src/bin/listener.rs
:language: rust
:start-after: snippet:start listener
:end-before: snippet:end listener
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/python/listener.py
:language: python
:start-after: snippet:start listener
:end-before: snippet:end listener
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/cxx/src/listener.cpp
:language: c++
:start-after: snippet:start listener
:end-before: snippet:end listener
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/c/src/listener.c
:language: c
:start-after: snippet:start listener
:end-before: snippet:end listener
:dedent:
```
````

Each event maps to an action. We wrap that in a small handler:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/event/rust/src/bin/listener.rs
:language: rust
:start-after: snippet:start react
:end-before: snippet:end react
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/python/listener.py
:language: python
:start-after: snippet:start react
:end-before: snippet:end react
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/cxx/src/listener.cpp
:language: c++
:start-after: snippet:start react
:end-before: snippet:end react
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/c/src/listener.c
:language: c
:start-after: snippet:start react
:end-before: snippet:end react
:dedent:
```
````

This participant doesn’t need to poll. It just blocks until an event arrives,
then runs the handler for each pending notification:

````{tab-set-code}
```{literalinclude} ../../../snippets/robot-nervous-system/event/rust/src/bin/listener.rs
:language: rust
:start-after: snippet:start wait-loop
:end-before: snippet:end wait-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/python/listener.py
:language: python
:start-after: snippet:start wait-loop
:end-before: snippet:end wait-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/cxx/src/listener.cpp
:language: c++
:start-after: snippet:start wait-loop
:end-before: snippet:end wait-loop
:dedent:
```

```{literalinclude} ../../../snippets/robot-nervous-system/event/c/src/listener.c
:language: c
:start-after: snippet:start wait-loop
:end-before: snippet:end wait-loop
:dedent:
```
````

## Related Examples

<!-- markdownlint-disable-next-line MD033 -->
<br/>

````{grid} 1 1 2 2
:gutter: 3

```{grid-item}
**{octicon}`code` Rust**

{octicon}`mark-github` [Minimal Notifier](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/event/notifier.rs)  
{octicon}`mark-github` [Minimal Listener](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/rust/event/listener.rs)
```

```{grid-item}
**{octicon}`code` Python**

{octicon}`mark-github` [Minimal Notifier](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/event/notifier.py)  
{octicon}`mark-github` [Minimal Listener](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/python/event/listener.py)
```

```{grid-item}
**{octicon}`code` C++**

{octicon}`mark-github` [Minimal Notifier](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/event/src/notifier.cpp)  
{octicon}`mark-github` [Minimal Listener](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/cxx/event/src/listener.cpp)
```

```{grid-item}
**{octicon}`code` C**

{octicon}`mark-github` [Minimal Notifier](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/event/src/notifier.c)  
{octicon}`mark-github` [Minimal Listener](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/c/event/src/listener.c)
```

````
