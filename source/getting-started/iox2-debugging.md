# Command Line Tools And Debugging

Now we already have quite a complex system. We have a ultra-sonic sensor
measuring the distance to obstacles, an emergency brake and auto pilot service
and a global config service.
If something does not work as expected we might want to see what is going on
inside the system, and check the healthyness of the nodes, the services.

When a sensor malfunctions we want to be able to see the communication data,
and when we discovered a bug in our algorithm we want to record the data that
caused the malfunction and replay it, so that we are sure that the problem is
fixed.

Luckily, iceoryx2 comes with some command line tooling to do all of this. It
can be installed with

```sh
cargo install iceoryx2-cli
```

It follows the same grammar as `cargo` for instance does. The first argument
is the subcommand and every subcommand comes with its own help and grammar
again. To see what subcommands are available we can call

```sh
iox2 --list
```

## Discovery

Let's assume our system is running and we want to get an overview of all the
nodes. A call to

```sh
iox2 node list
```

prints us a full list of all nodes currently active. With

```sh
iox2 node details 00000000067ab750000005550000dcf7
```

we see the full configuration of a node and its state - if it is dead or alive.
The long number is the node id, which was displayed in the `iox2 node list`
command.

Services can be discovered as well with

```sh
iox2 service list
```

## Debugging

### Data Introspection

Let's assume we encounter a bug where our robot Larry does not break despite
an obstacle is directly in front of our ultra-sonic sensor. The first low-level
thing we can do, is to check if the service is sending data and what kind of
data.

We can subscribe also to any kind of service via the command line and see the
payload hex-encoded.

```sh
iox2 service subscribe "distance_to_obstacle"
```

If everything seems to be in order, the service may not send out the
notifications to the emergency break. We can check this as well with

```sh
iox2 service listen "distance_to_obstacle"
```

### Injecting Data

So everything seems to be send as it should be. Now it is time to check the
emergency brake process directly and send a notification to it directly. In our
example, the event id `5` signaled that the obstacle was too close. Lets send it
ten times directly and check if the emergency brake is activated.

```sh
iox2 service notify --event-id 5 --num 10 distance_to_obstacle
```

Still no luck, alright then we have to manually publish the data via command
line and then notify the emergency brake again. If it then does not activate
we might have a problem in our algorithm. On the command line we have to
manually define the type and provide the payload as hex code. The ultra sonic
sensor is publishing the following type:

````{tab-set-code}
```{code-block} rust
#[derive(Debug, ZeroCopySend)]
#[repr(C)]
pub struct Distance {
    pub distance_in_meters: f64,
    pub some_other_property: f32,
}
```

```{code-block} python
import ctypes

class TransmissionData(ctypes.Structure):
    _fields_ = [
        ("distance_in_meters", ctypes.c_double),
        ("some_other_property", ctypes.c_float),
    ]
```

```{code-block} c++
struct Distance {
    double distance_in_meters;
    float some_other_property;
};
```

```{code-block} c
struct Distance {
    double distance_in_meters;
    float some_other_property;
};
```
````

We send out the same message 10 times and provide the hex content.

```sh
iox2 service publish -d IOX2DUMP --type-name Distance --type-size 16 \
    --type-alignment 8 --type-variant FIXEDSIZE --repetitions 10 \
    --message "09 00 00 00 1b 00 00 00 ae 47 e1 7a 14 8d bc 40" \
    distance_to_obstacle
```

### Record And Replay

When we found out that we had a bug in our algorithm, we need now to write an
integration test, where we provide sensor input for the emergency brake and we
test that the emergency brake really activates. In a normal desktop application
we would write first a unit test, fix the bug and then the out unit test passes
and proofs that the bug was fixed.

In iceoryx2 we can record data that causes the bug, fix the bug and then replay
the pre-recorded data. If the bug no longer shows we have proven that the issue
is fixed. We start recording the data into the file `record.dat` for the next 10
seconds and at that time we place an obstacle in front of the ultra-sonic sensor
so that the emergency brake should activate.

```sh
iox2 service record -t 10 -o record.dat "distance_to_obstacle"
```

We can take a look into the file `record.dat` it is by default human readable.
The file starts with the service header that defines the header, user header
and payload type as well as the service name, messaging pattern and the iceoryx2
version.

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

[details.types.user_header]
variant = "FixedSize"
type_name = "()"
size = 0
alignment = 1

[details.types.system_header]
variant = "FixedSize"
type_name = "iceoryx2::service::header::publish_subscribe::Header"
size = 40
alignment = 8
```

After the header, the actual recording follows. The payload is encoded in
hex-code and can also be modified later for debugging purposes.

```
### Recorded Data Start ###
time:     925
sys head: c0 f3 00 00 76 0b 00 00 44 d1 f5 2d 00 00 00 00 c0 f3 00 00 76 0b 00 00 36 d4 03 2e 01 00 00 00 01 00 00 00 00 00 00 00
usr head:
payload:  21 00 00 00 63 00 00 00 0a d7 a3 70 fd 2b da 40

time:     1925
sys head: c0 f3 00 00 76 0b 00 00 44 d1 f5 2d 00 00 00 00 c0 f3 00 00 76 0b 00 00 36 d4 03 2e 01 00 00 00 01 00 00 00 00 00 00 00
usr head:
payload:  22 00 00 00 66 00 00 00 ec 51 b8 1e 05 f7 da 40
```

When the bug is fixed we can replay our recorded `record.dat` file and see if
the system behaves as we expect it.

```sh
iox2 service replay -i record.dat
```



# Command Line Tools and Debugging

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

During that time, place an obstacle in front of Larry so the brake *should*
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
