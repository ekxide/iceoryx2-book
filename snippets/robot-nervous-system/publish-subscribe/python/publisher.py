"""Ultrasonic sensor distance publisher."""

from distance import Distance


def get_ultra_sonic_sensor_distance() -> float:
    """Stand in for the platform-specific ultrasonic sensor read."""
    return 0.42


# snippet:start node
import iceoryx2 as iox2

node = (
    iox2.NodeBuilder.new()
    .name(iox2.NodeName.new("UltraSonicSensor"))
    .create(iox2.ServiceType.Ipc)
)
# snippet:end node

# snippet:start service
service = (
    node.service_builder(iox2.ServiceName.new("distance_to_obstacle"))
    .publish_subscribe(Distance)
    .open_or_create()
)
# snippet:end service

# snippet:start publisher
publisher = service.publisher_builder().create()
# snippet:end publisher

# snippet:start publish-loop
try:
    while True:
        node.wait(iox2.Duration.from_millis(100))

        sample = publisher.loan_uninit()
        distance = get_ultra_sonic_sensor_distance()
        sample = sample.write_payload(
            Distance(distance_in_meters=distance, some_other_property=42.0)
        )
        sample.send()

except iox2.NodeWaitFailure:
    print("exit")
# snippet:end publish-loop
