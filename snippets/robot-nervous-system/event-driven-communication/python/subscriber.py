"""Event-driven distance subscriber."""

from distance import Distance


def go_into_parking_position() -> None:
    """Stand in for Larry's parking maneuver."""


def perform_break(last_samples) -> None:
    """Stand in for the emergency-brake computation."""


# snippet:start setup
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

ultra_sonic_service_dead = iox2.EventId.new(10)
event_service = (
    node.service_builder(iox2.ServiceName.new("distance_to_obstacle"))
    .event()
    .notifier_dead_event(ultra_sonic_service_dead)
    .open_or_create()
)

subscriber = pubsub_service.subscriber_builder().create()
listener = event_service.listener_builder().create()
# snippet:end setup

obstacle_too_close = iox2.EventId.new(5)

# snippet:start wait-loop
try:
    while True:
        sensor_is_dead = False
        obstacle_detected = False

        for event in listener.blocking_wait():
            if event.id == ultra_sonic_service_dead:
                sensor_is_dead = True
            if event.id == obstacle_too_close:
                obstacle_detected = True

        if sensor_is_dead:
            go_into_parking_position()

        if obstacle_detected:
            last_samples = []
            while len(last_samples) < 3:
                sample = subscriber.receive()
                if sample is None:
                    break
                last_samples.append(sample)
            perform_break(last_samples)

except iox2.ListenerWaitError:
    print("exit")
# snippet:end wait-loop
