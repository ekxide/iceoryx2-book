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

Targeting safety-critical systems, iceoryx2 assumes worst-case scenarios for determining memory allocation. Qos play a crucial role in these calculations - using smaller numbers in various settings can reduce the memory consumption significantly ...
Describe how publisher, client and server data segments can be shrinked.

## Specify Alignment

- for specialized use cases, like SIMD or FPGA, one can define custom alignments for the service payload

## Further Reading

- Fundamentals - Messaging Patterns - PubSub/Events/ReqRes?
- Getting Started - A Robot Nervous System - PubSub/Events/ReqRes?

TODO:
- change qos link in Fundamentals/Communication Model
- link to this tutorial from other "Further Reading" sections
- check if previous tutorial shows this tutorial at "Next"
