# Request-Response

```{figure} /images/request-response-pattern.svg
:alt: request-response messaging pattern
:align: center
:name: fig-request-response-communication

Request-Response Communication Model
```

A bi-directional messaging pattern that enables direct communication
between participants. Clients can send requests to servers through a
service, with each individual request capable of receiving one or multiple
streaming responses.

The request-response messaging pattern is ideal for scenarios requiring
immediate feedback, data queries, or remote procedure calls. The use of shared
memory enables efficient transfer of both request and response payloads.

The pattern is not suitable for cases where one-way communication suffices
([publish-subscribe](/fundamentals/messaging-patterns/publish-subscribe)) or
when data needs to persist for multiple consumers ([blackboard](
/fundamentals/messaging-patterns/blackboard)).

## Mechanism

```{figure} /images/request-response-mechanism.svg
:alt: request-response messaging protocol
:align: center
:name: fig-request-response-mechanism

Request-Response over Shared Memory
```

The `RequestResponse` messaging pattern is implemented using data structures in
shared memory. Separate shared memory segments are used for sending requests
and responses. The following structures are involved:

1. **Payload segment**
    * A region in shared memory used to communicate payload data between
      participants
    * Clients have a payload segment to share requests with servers
    * Servers have a payload segment to share responses with clients
    * The organization of memory in the payload segment depends on the
      allocator used in the `iceoryx2` deployment
    * Deployments for desktop applications may prefer a different strategy to
      safety-critical applications
1. **Offset Channel**
    * A channel for communicating the offsets of payloads between participants
    * Each client-server pair has one offset channel for requests and one
      offset channel for responses

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Understand Events
:link: /fundamentals/messaging-patterns/event
:link-type: doc

See how the `Event` messaging pattern can be used to coordinate participants.
```

```{grid-item-card} Example: Request-Response Service (Rust)
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/rust/request_response
:link-type: url

See how to set up a basic `RequestResponse` service.
```

```{grid-item-card} Example: Dynamic Request-Response Service (Rust)
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/rust/request_response_dynamic_data
:link-type: url

See how to set up a `RequestResponse` service with dynamic memory allocations.
```

````
