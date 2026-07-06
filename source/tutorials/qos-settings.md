# Quality of Service Settings

```{admonition} Learning Objectives
TODO: Start with Learning Objectives.
```

Remember Larry, our imaginary robot, that helped us to get familiar with iceoryx2? As many robotics projects, Larry started quite simple, just a camera and an ultrasonic sensor. But real robotics systems usually grow fast - more sensors, more algorithms, more real-time constraints, and suddenly it needs to run on a resource-constrained embedded hardware. Now we have more advanced algorithms, some that need always the latest data, others that must not lose any samples. Participants can crash and we want to get notified so that we can enter a safe fallback. Additionally, we must consider memory consumption. This is where the quality of service (qos) settings come to help. An iceoryx2 service is defined by a unique service name, a messaging pattern, and the qos. When creating a service, a participant defines the terms of communication by specifying the messaging pattern and the qos properties. When opening a service, a participant may specify its own qos. These settings are interpreted as minimum requirements that must be compatible in order for communication to occur.

## All or Latest Data?

- see Event-Driven Communication article
- example ultrasonic sensor starts before emergency brake; the latter needs the most recent three samples to compute position, relative speed, accelaration
- explain how subscriber_max_buffer_size, history_size, subscriber_max_borrowed_samples interact
- example for not losing any data...
- overflow behavior? will be refactored...
- req/res equivalents

## Get Notified

- get notified on certain events
- need to wake up a function with no additional input/need several inputs and you don't want to be interrupted until all of them have arrived
- example emergency brake system: it does not need every single distance sample from the ultrasonic sensor; only care when the object is close enough + approaching
- notifier_dead_event, so that emergency brake can switch to a safe state (ensures the system can respond effectively to crashes by notifying the corresponding listeners) (see Event-Driven Communication example)
- others...

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

- for specialized use cases, like SIMD or FPGA, one can define custom alignments for the service payload

## Further Reading

- Fundamentals - Messaging Patterns - PubSub/Events/ReqRes?
- Getting Started - A Robot Nervous System - PubSub/Events/ReqRes?

TODO:
- change qos link in Fundamentals/Communication Model
- link to this tutorial from other "Further Reading" sections
- check if previous tutorial shows this tutorial at "Next"
- provide code snippets
