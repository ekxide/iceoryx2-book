use core::time::Duration;
use publish_subscribe::distance::Distance;

// stands in for the platform-specific ultrasonic sensor read
fn get_ultra_sonic_sensor_distance() -> f64 {
    0.42
}

fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start node
    use iceoryx2::prelude::*;

    let node = NodeBuilder::new()
        .name(&"UltraSonicSensor".try_into()?)
        .create::<ipc::Service>()?;
    // snippet:end node

    // snippet:start service
    let service = node
        .service_builder(&"distance_to_obstacle".try_into()?)
        .publish_subscribe::<Distance>()
        .open_or_create()?;
    // snippet:end service

    // snippet:start publisher
    let publisher = service.publisher_builder().create()?;
    // snippet:end publisher

    // snippet:start publish-loop
    while node.wait(Duration::from_millis(100)).is_ok() {
        let sample = publisher.loan_uninit()?;

        let sample = sample.write_payload(Distance {
            distance_in_meters: get_ultra_sonic_sensor_distance(),
            some_other_property: 42.0,
        });

        sample.send()?;
    }
    // snippet:end publish-loop

    Ok(())
}
