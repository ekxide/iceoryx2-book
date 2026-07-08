# Quality of Service Settings

```{admonition} Learning Objectives
TODO: Start with Learning Objectives.
```

Remember Larry, our imaginary robot, that helped us to get familiar with iceoryx2? As many robotics projects, Larry started quite simple, just a camera and an ultrasonic sensor. But real robotics systems usually grow fast - more sensors, more algorithms, more real-time constraints, and suddenly it needs to run on a resource-constrained embedded hardware. Now we have more advanced algorithms, some that need always the latest data, others that must not lose any samples. Participants can crash and we want to get notified so that we can enter a safe fallback. Additionally, we must consider memory consumption. This is where the quality of service (qos) settings come to help. An iceoryx2 service is defined by a unique service name, a messaging pattern, and the qos. When creating a service, a participant defines the terms of communication by specifying the messaging pattern and the qos properties. When opening a service, a participant may specify its own qos. These settings are interpreted as minimum requirements that must be compatible in order for communication to occur.

## All or Latest Data?

### Pub-Sub

- subscriber needs to hold three samples to compute position, relative speed, accelaration:
  - subscriber_max_borrowed_samples: defines how many samples a subscriber can borrow at most in parallel
- subscriber needs the last three samples whenever it connects, could start after sensor participant:
  - subscriber_max_buffer_size: defines how many samples a subscriber can store in its internal buffer
  - history_size: defines the maximum history size a subscriber can request on connection
  - if overflow is not enabled: history_size <= subscriber_max_buffer_size

- there could be other subscribers to the same service with less requirements. that's why one can set buffer_size and history_request on subscriber port creation allowing to **lower** expectations on buffer size and history for this specific subscriber
  - buffer_size: a subscriber can decrease the service's subscriber_max_buffer_size (buffer_size <= subscriber_max_buffer_size)
  - history_request: defines the amount of requested history samples (history_request <= service's history_size; history_request <= buffer_size)

- overflow behavior (will be refactored soon):
  - enabled: subscriber receives the most recent samples. i.e. when subscriber buffer is full, the oldest sample will be discarded and the most recent one is received
  - disabled: the subscriber receives as many samples as fit in its buffer. samples that are sent after the buffer is full, will be discarded on subscriber side. if you don't want to lose data, disable overflow and choose buffer size accordingly

- example: subscriber with buffer size 10 and history size 4. a publisher sends 10 samples before the subscriber connects. the subscriber receives the most recent 4 samples when it connects. after receiving, the publisher sends 10 more samples without the connected subscriber receiving them. when:
  - overflow is enabled: the subscriber receives the 10 most recent samples; all other samples are discarded
  - overflow is disabled: the subscriber receives 6 samples; the 4 most recent samples are discarded

### Req-Res

- max_response_buffer_size: defines how many responses fit in the client's buffer per request; important when a response stream is expected
- overflow behavior for requests and responses (will be refactored soon)

## Get Notified

Service settings:

- deadline: defines how long a listener has to wait at most until a signal will be received; there must be a notification emitted by any notifier after at least the deadline; defines the maximum allowed time between two consecutive notifications. if a notification is not sent after the defined time, every listener that is attached to a waitset will be notified
- disable_deadline: disable the deadline property of the service (may set by config); notifiers can signal notifications at any rate
- max_nodes: defines how many nodes the service does support; defines indirectly how many processes can open the service at the same time
- event_id_max_value: defines the maximum supported event id value; (an increased value can have a significant performance impact on some configurations that use a bitset as event tracking mechanism)
- max_notifiers: defines how many notifiers an event service supports
- max_listeners: defines how many listeners an event service supports
- notifier_created_event: optional event id that is emitted when a new notifier was created; defines the event that shall be emitted by every newly created notifier
- disable_notifier_created_event: disable the above
- notifier_dropped_event: optional event id that is emitted when a notifier is dropped; defines the event that shall be emitted by every notifier before it is dropped
- disable_notifier_dropped_event: disables the above
- notifier_dead_event: optional event id that is emitted when a notifier is identified as dead (e.g. allowing the emergency brake to switch to a safe state when the sensor participant is identified as dead)
- disable_notifier_dead_event: disables the above

Notifier port settings:

- notifier port setting default_event_id: sets a default event id for this notifier that is used in notify() (all listeners connected to the service with the default event id are notified <-> notify_with_custom_event_id(EventId))

## Reduce Memory Consumption

Targeting safety-critical systems, iceoryx2 assumes worst-case scenarios for determining memory allocation. Various qos settings play a crucial role in these calculations - using smaller numbers can reduce the memory consumption significantly.

- reduce size of payload segment in shm (link to Fundamentals/Shared Memory?)
- relevant for pub-sub (publisher, link to Fundamentals/Messaging Patterns/ Publsih-Subscribe?) and req-res (client, server, link to Fundamentals/Messaging Patterns/Request-Response?)

### Pub-Sub

**Publisher payload segment:**

1. Service settings:

- max_subscribers: defines how many subscribers a publish-subscribe service supports
- subscriber_max_buffer_size: defines how many samples a subscriber can store in its internal buffer
- subscriber_max_borrowed_samples: defines how many samples a subscriber can borrow at most in parallel
- history_size: defines the maximum history size a subscriber can request on connection

2. Publisher port settings:

- max_loaned_samples: defines how many SampleMuts this specific publisher can loan in parallel
- override_sample_preallocation: see port_factory/publisher.rs

### Req-Res

**Client payload segment:**

1. Service settings:

- max_servers: defines how many servers a request-response service supports
- max_loaned_requests: defines how may requests a client can loan in parallel (in total for all servers)
- max_active_requests_per_client: defines how many active requests (= objects used to send answers to a
  request that was received earlier from a client) a server can hold in parallel

2. Port settings:

- client: max_active_requests: max amount of active requests this specific client can send
- client: override_request_preallocation: see port_factory/client.rs

**Server payload segment:**

1. Service settings:

- max_clients: defines how many clients a request-response service supports
- max_active_requests_per_client: defines how many active requests (= objects used to send answers to a
  request that was received earlier from a client) a server can hold in parallel

=> max_clients * 2 * max_active_requests_per_client = number of active requests in total for all clients (worst case)

- max_borrowed_responses_per_pending_response: max borrowed responses a client can hold in parallel per pending
  response (= objects used by the clients to receive responses for earlier sent requests)
- max_response_buffer_size: defines how many responses fit in the client's buffer per request; important when a
  response stream is expected

2. Port settings:

- server: max_loaned_responses_per_request: defines how many responses this specific server can loan in parallel per
  active request
- server: override_response_preallocation: see port_factory/server.rs

## Specify Alignment

- some libraries (especially SIMD-related code) require stricter alignment than
the type used for communication provides
- in those cases, one can increase the payload alignment:

  Pub-Sub:

  - payload_alignment: defines the alignment of the payload for the service. if the provided alignment is greater than the payload type's alignment, the latter is used.

  Req-Res:
  - request_payload_alignment: overrides the alignment of the request payload
  - response_payload_alignment: overrides the alignment of the response payload

- to be able to connect to a service, the payload alignment must be identical in all participants since the communication is always strongly typed


## Further Reading

- Fundamentals - Messaging Patterns - PubSub/Events/ReqRes?
- Getting Started - A Robot Nervous System - PubSub/Events/ReqRes?

TODO:
- change qos link in Fundamentals/Communication Model
- link to this tutorial from other "Further Reading" sections
- check if previous tutorial shows this tutorial at "Next"
- provide code snippets
- degradation_handler?
- link to default settings?
