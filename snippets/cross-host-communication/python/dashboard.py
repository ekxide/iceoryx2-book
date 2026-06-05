"""Dashboard subscriber."""

# snippet:start
import iceoryx2 as iox2
from telemetry_data import BatteryState, Position

iox2.set_log_level_from_env_or(iox2.LogLevel.Info)
node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)

battery_subscriber = (
    node.service_builder(iox2.ServiceName.new("larry/battery"))
    .publish_subscribe(BatteryState)
    .open_or_create()
    .subscriber_builder()
    .create()
)
battery_listener = (
    node.service_builder(iox2.ServiceName.new("larry/battery"))
    .event()
    .open_or_create()
    .listener_builder()
    .create()
)

position_subscriber = (
    node.service_builder(iox2.ServiceName.new("larry/position"))
    .publish_subscribe(Position)
    .open_or_create()
    .subscriber_builder()
    .create()
)
position_listener = (
    node.service_builder(iox2.ServiceName.new("larry/position"))
    .event()
    .open_or_create()
    .listener_builder()
    .create()
)

waitset = iox2.WaitSetBuilder.new().create(iox2.ServiceType.Ipc)
battery_guard = waitset.attach_notification(battery_listener)
position_guard = waitset.attach_notification(position_listener)

try:
    while True:
        attachments, result = waitset.wait_and_process()
        if result in (
            iox2.WaitSetRunResult.TerminationRequest,
            iox2.WaitSetRunResult.Interrupt,
        ):
            break

        for attachment_id in attachments:
            if attachment_id.has_event_from(battery_guard):
                # drain every pending notification, otherwise the WaitSet wakes
                # again immediately and spins
                battery_listener.try_wait_all()
                sample = battery_subscriber.receive()
                while sample is not None:
                    print("battery:", sample.payload().contents.charge_percent, "%")
                    sample = battery_subscriber.receive()
            elif attachment_id.has_event_from(position_guard):
                # drain every pending notification, otherwise the WaitSet wakes
                # again immediately and spins
                position_listener.try_wait_all()
                sample = position_subscriber.receive()
                while sample is not None:
                    position = sample.payload().contents
                    print("position:", (position.x, position.y))
                    sample = position_subscriber.receive()

except iox2.WaitSetRunError:
    print("exit")
# snippet:end
