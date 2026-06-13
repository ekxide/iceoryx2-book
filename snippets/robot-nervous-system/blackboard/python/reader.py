"""Blackboard configuration reader."""

import ctypes

from distance import Distance


def get_ultra_sonic_sensor_distance() -> float:
    """Stand in for the platform-specific ultrasonic sensor read."""
    return 0.42


import iceoryx2 as iox2

node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)

# the distance publisher is reused from the publish-subscribe example
publisher = (
    node.service_builder(iox2.ServiceName.new("distance_to_obstacle"))
    .publish_subscribe(Distance)
    .open_or_create()
    .publisher_builder()
    .create()
)

us_sensor_key = ctypes.c_uint64(1)

# snippet:start service
service = (
    node.service_builder(iox2.ServiceName.new("global_config"))
    .blackboard_opener(ctypes.c_uint64)
    .open()
)
# snippet:end service

# snippet:start reader
reader = service.reader_builder().create()
# snippet:end reader

# snippet:start handle
update_rate_handle = reader.entry(us_sensor_key, ctypes.c_uint32)
# snippet:end handle

# snippet:start sensor-loop
try:
    while True:
        node.wait(
            iox2.Duration.from_millis(
                update_rate_handle.get().decode_as(ctypes.c_uint32).value
            )
        )

        sample = publisher.loan_uninit()

        distance = get_ultra_sonic_sensor_distance()
        sample = sample.write_payload(
            Distance(distance_in_meters=distance, some_other_property=42.0)
        )

        sample.send()

except iox2.NodeWaitFailure:
    print("exit")
# snippet:end sensor-loop
