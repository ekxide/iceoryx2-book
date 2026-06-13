// stand-ins for the health-monitor's reactions
fn activate_battery_warning_light() {}

fn go_into_parking_position() {}

fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start node-and-service
    use iceoryx2::prelude::*;

    let node = NodeBuilder::new().create::<ipc::Service>()?;

    let event_service = node
        .service_builder(&"system_health_events".try_into()?)
        .event()
        .open_or_create()?;
    // snippet:end node-and-service

    // snippet:start listener
    let listener = event_service.listener_builder().create()?;
    // snippet:end listener

    let battery_is_low = EventId::new(1);
    let wall_was_hit = EventId::new(0);

    // snippet:start wait-loop
    while listener
        .blocking_wait(|event| {
            if event.id == battery_is_low {
                activate_battery_warning_light();
            }
            if event.id == wall_was_hit {
                go_into_parking_position();
            }
        })
        .is_ok()
    {}
    // snippet:end wait-loop

    Ok(())
}
