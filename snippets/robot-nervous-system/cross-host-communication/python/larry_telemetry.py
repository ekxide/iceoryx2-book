"""Larry telemetry publisher."""

# snippet:start
import iceoryx2 as iox2
from telemetry_data import BatteryState, Position

cycle_time = iox2.Duration.from_secs(1)

iox2.set_log_level_from_env_or(iox2.LogLevel.Info)
node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)

battery_publisher = (
    node.service_builder(iox2.ServiceName.new("larry/battery"))
    .publish_subscribe(BatteryState)
    .open_or_create()
    .publisher_builder()
    .create()
)
battery_notifier = (
    node.service_builder(iox2.ServiceName.new("larry/battery"))
    .event()
    .open_or_create()
    .notifier_builder()
    .create()
)

position_publisher = (
    node.service_builder(iox2.ServiceName.new("larry/position"))
    .publish_subscribe(Position)
    .open_or_create()
    .publisher_builder()
    .create()
)
position_notifier = (
    node.service_builder(iox2.ServiceName.new("larry/position"))
    .event()
    .open_or_create()
    .notifier_builder()
    .create()
)

try:
    while True:
        node.wait(cycle_time)

        battery = battery_publisher.loan_uninit()
        battery = battery.write_payload(BatteryState(charge_percent=87.0))
        battery.send()
        battery_notifier.notify()

        position = position_publisher.loan_uninit()
        position = position.write_payload(Position(x=1.0, y=2.0))
        position.send()
        position_notifier.notify()

except iox2.NodeWaitFailure:
    print("exit")
# snippet:end
