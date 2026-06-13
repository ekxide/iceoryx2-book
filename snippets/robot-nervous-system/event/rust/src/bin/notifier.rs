use core::time::Duration;

// stand-ins for Larry's sensor hardware
fn bump_sensor_was_activated() -> bool {
    false
}

fn battery_state() -> f32 {
    100.0
}

fn main() -> Result<(), Box<dyn core::error::Error>> {
    let battery_threshold = 20.0;

    // snippet:start node-and-service
    use iceoryx2::prelude::*;

    let node = NodeBuilder::new().create::<ipc::Service>()?;

    let event_service = node
        .service_builder(&"system_health_events".try_into()?)
        .event()
        .open_or_create()?;
    // snippet:end node-and-service

    // snippet:start notifier
    let notifier = event_service.notifier_builder().create()?;
    // snippet:end notifier

    // snippet:start event-ids
    let wall_was_hit = EventId::new(0);
    let battery_is_low = EventId::new(1);
    // snippet:end event-ids

    // snippet:start notify-loop
    while node.wait(Duration::from_secs(1)).is_ok() {
        if bump_sensor_was_activated() {
            notifier.notify_with_custom_event_id(wall_was_hit)?;
        }

        if battery_state() < battery_threshold {
            notifier.notify_with_custom_event_id(battery_is_low)?;
        }
    }
    // snippet:end notify-loop

    Ok(())
}
