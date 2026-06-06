fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start
    use core::time::Duration;
    use cross_host_communication::telemetry_data::{BatteryState, Position};
    use iceoryx2::prelude::*;

    const CYCLE_TIME: Duration = Duration::from_secs(1);

    let node = NodeBuilder::new()
        .name(&"Larry".try_into()?)
        .create::<ipc::Service>()?;

    let battery_publisher = node
        .service_builder(&"larry/battery".try_into()?)
        .publish_subscribe::<BatteryState>()
        .open_or_create()?
        .publisher_builder()
        .create()?;
    let battery_notifier = node
        .service_builder(&"larry/battery".try_into()?)
        .event()
        .open_or_create()?
        .notifier_builder()
        .create()?;

    let position_publisher = node
        .service_builder(&"larry/position".try_into()?)
        .publish_subscribe::<Position>()
        .open_or_create()?
        .publisher_builder()
        .create()?;
    let position_notifier = node
        .service_builder(&"larry/position".try_into()?)
        .event()
        .open_or_create()?
        .notifier_builder()
        .create()?;

    while node.wait(CYCLE_TIME).is_ok() {
        let battery = battery_publisher.loan_uninit()?;
        let battery = battery.write_payload(BatteryState {
            charge_percent: 87.0,
        });
        battery.send()?;
        battery_notifier.notify()?;

        let position = position_publisher.loan_uninit()?;
        let position = position.write_payload(Position { x: 1.0, y: 2.0 });
        position.send()?;
        position_notifier.notify()?;
    }
    // snippet:end

    Ok(())
}
