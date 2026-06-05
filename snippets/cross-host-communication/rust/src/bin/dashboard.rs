fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start
    use cross_host_communication::telemetry_data::{BatteryState, Position};
    use iceoryx2::prelude::*;

    let node = NodeBuilder::new()
        .name(&"Dashboard".try_into()?)
        .create::<ipc::Service>()?;

    let battery_subscriber = node
        .service_builder(&"larry/battery".try_into()?)
        .publish_subscribe::<BatteryState>()
        .open_or_create()?
        .subscriber_builder()
        .create()?;
    let battery_listener = node
        .service_builder(&"larry/battery".try_into()?)
        .event()
        .open_or_create()?
        .listener_builder()
        .create()?;

    let position_subscriber = node
        .service_builder(&"larry/position".try_into()?)
        .publish_subscribe::<Position>()
        .open_or_create()?
        .subscriber_builder()
        .create()?;
    let position_listener = node
        .service_builder(&"larry/position".try_into()?)
        .event()
        .open_or_create()?
        .listener_builder()
        .create()?;

    let waitset = WaitSetBuilder::new().create::<ipc::Service>()?;
    let battery_guard = waitset.attach_notification(&battery_listener)?;
    let position_guard = waitset.attach_notification(&position_listener)?;

    let on_event = |attachment_id: WaitSetAttachmentId<ipc::Service>| {
        if attachment_id.has_event_from(&battery_guard) {
            // drain every pending notification, otherwise the WaitSet wakes
            // again immediately and spins
            battery_listener.try_wait_all(|_| {}).unwrap();
            while let Ok(Some(sample)) = battery_subscriber.receive() {
                println!("battery: {}%", sample.charge_percent);
            }
        } else if attachment_id.has_event_from(&position_guard) {
            // drain every pending notification, otherwise the WaitSet wakes
            // again immediately and spins
            position_listener.try_wait_all(|_| {}).unwrap();
            while let Ok(Some(sample)) = position_subscriber.receive() {
                println!("position: ({}, {})", sample.x, sample.y);
            }
        }

        CallbackProgression::Continue
    };

    waitset.wait_and_process(on_event)?;
    // snippet:end

    Ok(())
}
