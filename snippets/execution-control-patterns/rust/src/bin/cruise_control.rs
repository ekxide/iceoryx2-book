fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start
    use core::time::Duration;
    use iceoryx2::prelude::*;

    const CONTROL_PERIOD: Duration = Duration::from_millis(20); // 50 Hz
    const SETPOINT_KMH: f32 = 100.0;
    const KP: f32 = 0.1;

    let node = NodeBuilder::new()
        .name(&"CruiseController".try_into()?)
        .create::<ipc::Service>()?;

    let speed_service = node
        .service_builder(&"vehicle/speed".try_into()?)
        .publish_subscribe::<f32>()
        .subscriber_max_buffer_size(1)
        .open_or_create()?;

    let throttle_service = node
        .service_builder(&"vehicle/throttle".try_into()?)
        .publish_subscribe::<f32>()
        .open_or_create()?;

    let speed_subscriber = speed_service.subscriber_builder().create()?;
    let throttle_publisher = throttle_service.publisher_builder().create()?;

    let mut current_speed = 0.0f32;

    while node.wait(CONTROL_PERIOD).is_ok() {
        match speed_subscriber.receive() {
            Ok(Some(sample)) => current_speed = *sample,
            Ok(None) => { /* no new sample — reuse last known speed */ }
            Err(_) => { /* handle receive failure */ }
        }

        let throttle = (KP * (SETPOINT_KMH - current_speed)).clamp(0.0, 1.0);
        throttle_publisher.send_copy(throttle)?;
    }
    // snippet:end

    Ok(())
}
