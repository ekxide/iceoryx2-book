"""System health listener."""


def activate_battery_warning_light() -> None:
    """Stand in for the battery warning light."""


def go_into_parking_position() -> None:
    """Stand in for Larry's parking maneuver."""


# snippet:start node-and-service
import iceoryx2 as iox2

node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)

event = (
    node.service_builder(iox2.ServiceName.new("system_health_events"))
    .event()
    .open_or_create()
)
# snippet:end node-and-service

# snippet:start listener
listener = event.listener_builder().create()
# snippet:end listener

battery_is_low = iox2.EventId.new(1)
wall_was_hit = iox2.EventId.new(0)

# snippet:start wait-loop
try:
    while True:
        for event in listener.blocking_wait():
            if event.id == battery_is_low:
                activate_battery_warning_light()
            if event.id == wall_was_hit:
                go_into_parking_position()

except iox2.ListenerWaitError:
    print("exit")
# snippet:end wait-loop
