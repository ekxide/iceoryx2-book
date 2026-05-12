fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start
    use core::time::Duration;
    use iceoryx2::prelude::*;

    const CONTROL_PERIOD: Duration = Duration::from_millis(20); // 50 Hz
    const SPEED_DEADLINE: Duration = Duration::from_millis(100);

    let node = NodeBuilder::new()
        .name(&"VehicleSupervisor".try_into()?)
        .create::<ipc::Service>()?;

    let speed_signal = node
        .service_builder(&"vehicle/speed".try_into()?)
        .event()
        .open_or_create()?;

    let emergency_stop_signal = node
        .service_builder(&"vehicle/emergency_stop".try_into()?)
        .event()
        .open_or_create()?;

    let speed_listener = speed_signal.listener_builder().create()?;
    let emergency_stop_listener = emergency_stop_signal.listener_builder().create()?;

    let waitset = WaitSetBuilder::new().create::<ipc::Service>()?;
    let tick_guard = waitset.attach_interval(CONTROL_PERIOD)?;
    let speed_guard = waitset.attach_deadline(&speed_listener, SPEED_DEADLINE)?;
    let emergency_stop_guard = waitset.attach_notification(&emergency_stop_listener)?;

    waitset.wait_and_process(|attachment_id| {
        if attachment_id.has_event_from(&tick_guard) {
            // run periodic control step
        }
        if attachment_id.has_missed_deadline(&speed_guard) {
            // speed sensor went silent — engage fail-safe
        }
        if attachment_id.has_event_from(&speed_guard) {
            speed_listener
                .try_wait_all(|_id| { /* drain */ })
                .unwrap();
            // process new speed reading
        }
        if attachment_id.has_event_from(&emergency_stop_guard) {
            emergency_stop_listener
                .try_wait_all(|_id| { /* drain */ })
                .unwrap();
            // engage emergency stop
        }
        CallbackProgression::Continue
    })?;
    // snippet:end

    Ok(())
}
