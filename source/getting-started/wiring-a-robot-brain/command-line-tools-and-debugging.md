# Command Line Tools and Debugging

```{admonition} Learning Objectives

TODO
```

Our system has grown into something fairly complex: an ultrasonic sensor
measuring distances, an emergency brake, an autopilot service, and a global
configuration service. When things go wrong, and they will, we need ways to peek
inside the system. Maybe a node misbehaves, maybe a service is down, or maybe
Larry stubbornly refuses to brake even when a wall is right in front of him.

For these cases, iceoryx2 provides command line tooling. With it, you can
inspect nodes and services, subscribe to data, inject test messages, and even
record and replay entire sessions.

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

First, let’s see what’s running. To list all nodes:

```sh
iox2 node list
```

To get details on a specific node (configuration, state, whether it’s alive or
dead), pass its ID:

```sh
iox2 node details 00000000067ab750000005550000dcf7
```

That long hex number is the node ID, which you’ll see in the `node list` output.

You can also list all services:

```sh
iox2 service list
```

## Debugging

### Data Introspection

Back to Larry’s misbehavior: the obstacle is right there, yet the emergency
brake doesn’t trigger. Step one is to check whether the distance sensor service
is even publishing data.

Subscribe to the service and inspect the payload (shown hex-encoded):

```sh
iox2 service subscribe "distance_to_obstacle"
```

If the data looks fine, the problem might be that notifications aren’t reaching
the emergency brake. We can listen for those:

```sh
iox2 service listen "distance_to_obstacle"
```

### Injecting Data

Suppose the data and notifications both look normal, but the brake still doesn’t
react. We can bypass the sensor and poke the brake directly. Event ID `5`
signals “obstacle too close.” Let’s send that notification ten times:

```sh
iox2 service notify --event-id 5 --num 10 distance_to_obstacle
```

If Larry still doesn’t stop, maybe the algorithm ignores the event unless real
sensor data arrives. So we publish test data manually and notify again. For this
we need the data type:

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

Now we can publish a sample message ten times:

```sh
iox2 service publish -d IOX2DUMP --type-name Distance --type-size 16 \
    --type-alignment 8 --type-variant FIXEDSIZE --repetitions 10 \
    --message "09 00 00 00 1b 00 00 00 ae 47 e1 7a 14 8d bc 40" \
    distance_to_obstacle
```

### Record and Replay

Once we’ve found the bug, we want to make sure it stays fixed. In regular
desktop development you’d write a unit test, patch the bug, and rerun the test.
With iceoryx2 you can capture the problematic data, fix your algorithm, and then
replay the exact same scenario to confirm the fix.

Start by recording 10 seconds of sensor data into a file:

```sh
iox2 service record -t 10 -o record.dat "distance_to_obstacle"
```

During that time, place an obstacle in front of Larry so the brake _should_
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

After the header comes the actual data, with payloads encoded in hex:

```text
### Recorded Data Start ###
time:     925
sys head: ...
payload:  21 00 00 00 63 00 00 00 0a d7 a3 70 fd 2b da 40
```

When your fix is in place, replay the data:

```sh
iox2 service replay -i record.dat
```

If the emergency brake finally kicks in, congratulations: Larry lives to bump
another day.
