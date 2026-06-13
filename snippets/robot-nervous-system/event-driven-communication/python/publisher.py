"""Event-driven distance publisher."""

from distance import Distance


def get_ultra_sonic_sensor_distance() -> float:
    """Stand in for the platform-specific ultrasonic sensor read."""
    return 0.42


distance_threshold = 1.0

# snippet:start pubsub-service
import iceoryx2 as iox2

node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)

pubsub_service = (
    node.service_builder(iox2.ServiceName.new("distance_to_obstacle"))
    .publish_subscribe(Distance)
    .subscriber_max_buffer_size(3)
    .history_size(3)
    .subscriber_max_borrowed_samples(3)
    .open_or_create()
)
# snippet:end pubsub-service

# snippet:start event-service
ultra_sonic_service_dead = iox2.EventId.new(10)
event_service = (
    node.service_builder(iox2.ServiceName.new("distance_to_obstacle"))
    .event()
    .notifier_dead_event(ultra_sonic_service_dead)
    .open_or_create()
)
# snippet:end event-service

# snippet:start ports
publisher = pubsub_service.publisher_builder().create()
notifier = event_service.notifier_builder().create()

obstacle_too_close = iox2.EventId.new(5)
# snippet:end ports

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

        if distance < distance_threshold:
            notifier.notify_with_custom_event_id(obstacle_too_close)

except iox2.NodeWaitFailure:
    print("exit")
# snippet:end publish-loop
