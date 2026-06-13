"""Blackboard configuration writer."""

import ctypes


def get_battery_threshold():
    """Stand in for the user-facing configuration input."""
    return None


def get_update_rate():
    """Stand in for the user-facing configuration input."""
    return None


# snippet:start node
import iceoryx2 as iox2

node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)
# snippet:end node

# snippet:start service
battery_key = ctypes.c_uint64(0)
us_sensor_key = ctypes.c_uint64(1)

service = (
    node.service_builder(iox2.ServiceName.new("global_config"))
    .blackboard_creator(ctypes.c_uint64)
    .add(battery_key, ctypes.c_float(0.25))
    .add(us_sensor_key, ctypes.c_uint32(100))
    .create()
)
# snippet:end service

# snippet:start writer
writer = service.writer_builder().create()
# snippet:end writer

# snippet:start entries
battery_threshold_handle = writer.entry(battery_key, ctypes.c_float)
update_rate_handle = writer.entry(us_sensor_key, ctypes.c_uint32)
# snippet:end entries

# snippet:start update-loop
try:
    while True:
        node.wait(iox2.Duration.from_millis(100))

        new_battery_threshold = get_battery_threshold()
        if new_battery_threshold is not None:
            # small value -> simple copy API
            battery_threshold_handle.update_with_copy(
                ctypes.c_float(new_battery_threshold)
            )

        new_update_rate = get_update_rate()
        if new_update_rate is not None:
            update_rate_handle.update_with_copy(ctypes.c_uint32(new_update_rate))

except iox2.NodeWaitFailure:
    print("exit")
# snippet:end update-loop
