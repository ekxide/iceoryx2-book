"""System health notifier."""


def bump_sensor_was_activated() -> bool:
    """Stand in for Larry's bump sensor."""
    return False


def battery_state() -> float:
    """Stand in for Larry's battery gauge."""
    return 100.0


battery_threshold = 20.0

# snippet:start node-and-service
import iceoryx2 as iox2

iox2.set_log_level_from_env_or(iox2.LogLevel.Info)
node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)

event = (
    node.service_builder(iox2.ServiceName.new("system_health_events"))
    .event()
    .open_or_create()
)
# snippet:end node-and-service

# snippet:start notifier
notifier = event.notifier_builder().create()
# snippet:end notifier

# snippet:start event-ids
wall_was_hit = iox2.EventId.new(0)
battery_is_low = iox2.EventId.new(1)
# snippet:end event-ids

# snippet:start notify-loop
try:
    while True:
        node.wait(iox2.Duration.from_secs(1))
        if bump_sensor_was_activated():
            notifier.notify_with_custom_event_id(wall_was_hit)

        if battery_state() < battery_threshold:
            notifier.notify_with_custom_event_id(battery_is_low)

except iox2.NodeWaitFailure:
    print("exit")
# snippet:end notify-loop
