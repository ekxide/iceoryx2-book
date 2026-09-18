# Command Line Tools and Debugging

Our system has grown into something fairly complex: an ultrasonic sensor
measuring distances, an emergency brake, an autopilot service, and a global
configuration service. When things go wrong, and they will, we need the ability
to peek inside the system. Maybe a node misbehaves, maybe a service is down,
or maybe an algorithm is not behaving as expected.

To assist with debugging scenarious such as these, `iceoryx2` provides command
line tooling. With it, it's possible to inspect nodes and services, subscribe
to data, inject test messages, and even record and replay entire sessions.

Install the tools with:

```sh
cargo install iceoryx2-cli
```

The CLI follows a grammar similar to `cargo`: the first argument is a
subcommand, and each subcommand has its own help and options. To see the
available subcommands:

```sh
iox2 --list
```

## Discovery

To list all nodes with the CLI, you can use the command:

```sh
iox2 node list
```

You can also get the details of a specific node (configuration, state, whether
it's alive of dead) by passing the node name, id or pid (on supported
platforms) to the command:

```sh
iox2 node details 00000000067ab750000005550000dcf7
```

In this example the node id is used, depicted by the long hex number,
which can be found in the `node list` output.

To list all services, you can use the command:

```sh
iox2 service list
```

It's also possible to interact with services directly, and record and replay
data using the CLI, which we will now take a look at in some more detail.

## Debugging

### Introspecting Data

For this article, we will assume that Larry has a bug where he stubbornly
refuses to brake even when an obstacle is right in front of him.
First, we can check whether the distance sensor service is even publishing
data using the CLI.

We can subscribe to the service and inspect the payload (which is shown
hex-encoded):

```sh
iox2 service subscribe "distance_to_obstacle"
```

If the data appears to be flowing, we can check whether notifications are
reaching the emergency brake:

```sh
iox2 service listen "distance_to_obstacle"
```

### Injecting Data

Suppose the data and notifications both appear to be normal, but the brake
still doesn’t react. We can bypass the sensor and attempt to trigger the brake
directly. Assuming event ID `5` has been specified to indicate “obstacle too
close”, we can try sending that notification ten times:

```sh
iox2 service notify --event-id 5 --num 10 distance_to_obstacle
```

If Larry still doesn't stop, we can try publishing some test data manually
before sending the notification to check whether the algorithm only reacts
when data is present.

For this we need the data type and the hex representation of the sample we want
to send. For brevity, the approach for retrieving the hex representation is not
shown, but generated from the struct:

````{tab-set-code}
```rust
#[derive(Debug, ZeroCopySend)]
#[repr(C)]
pub struct Distance {
    pub distance_in_meters: f64,
    pub some_other_property: f32,
}
```

```python
import ctypes

class Distance(ctypes.Structure):
    _fields_ = [
        ("distance_in_meters", ctypes.c_double),
        ("some_other_property", ctypes.c_float),
    ]
```

```c++
struct Distance {
    double distance_in_meters;
    float some_other_property;
};
```

```c
struct Distance {
    double distance_in_meters;
    float some_other_property;
};
```
````

When we have the hex representation of a sample, we can publish it (ten times)
using this command:

```sh
iox2 service publish -d IOX2DUMP --type-name Distance --type-size 16 \
    --type-alignment 8 --type-variant FIXEDSIZE --repetitions 10 \
    --message "09 00 00 00 1b 00 00 00 ae 47 e1 7a 14 8d bc 40" \
    distance_to_obstacle
```

This mechanism allows for probing the system to try and zero in on what may
be causing the erroneous behaviour.

### Recording and Replaying Data

The CLI provides the capability to record and replay data flowing in the
`iceoryx2` system. This capability can be used to develop tests for
scenarios that cause erroneous behaviour.

Similar to test-driven development, where a unit test is written that captures
a bug, code is updated to handle it, and the bug is confirmed fixed when the
test passes; a recording of a scenario that causes misbehavior can be
captured, the code can be updated to handle it, and the fix can be confirmed by
replaying the scenario and observing that the system no longer exhibits
erroneous behavior.

The following CLI command will record 10 seconds of sensor data to a file:

```sh
iox2 service record -t 10 -o record.dat "distance_to_obstacle"
```

Returning to the theoretical bug being observed in Larry, we could capture
a recording of the scenario causing it by placing an obstacle in front of Larry
during the recording time, which is a situation where the brake _should_
activate.

The file `record.dat` is human-readable. It starts with metadata about the
service, messaging pattern, and type definitions:

```toml
service_name = "distance_to_obstacle"

[iceoryx2_version]
major = 0
minor = 6
patch = 1

[details]
file_format_version = 1
messaging_pattern = "PublishSubscribe"

[details.types.payload]
variant = "FixedSize"
type_name = "Distance"
size = 16
alignment = 8
```

After the header comes the payload data, encoded in hex:

```text
### Recorded Data Start ###
time:     925
sys head: ...
payload:  21 00 00 00 63 00 00 00 0a d7 a3 70 fd 2b da 40
```

When you have fixed the algorithm, you can then replay the data with the
following CLI command:

```sh
iox2 service replay -i record.dat
```

If the emergency brake kicks in, you have confidence that the algorithm
has been fixed: Larry lives to bump another day!
