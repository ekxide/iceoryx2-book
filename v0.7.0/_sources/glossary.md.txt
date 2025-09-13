# Glossary

```{glossary}
Unit of Execution
    The smallest schedulable entity that can be managed by the runtime environment.

Thread
    A single sequence of instructions that is the primary unit of execution on most systems.

Process
    A unit of execution with at least one thread and its own isolated virtual memory space and resource pool.

Participant
    A unit of execution participating in communication.

Concept
    Components with uniform abstract behaviour that provide domain-specific
    functionality.

Deployment
    A configurable set of concept implementations. Examples include
    [`Local`](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/iceoryx2/src/service/local.rs#L51)
    and [`Ipc`](https://github.com/eclipse-iceoryx/iceoryx2/blob/main/iceoryx2/src/service/ipc.rs#L50).
```
