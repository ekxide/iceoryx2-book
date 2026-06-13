use blackboard::distance::Distance;
use core::time::Duration;
use iceoryx2_bb_container::string::StaticString;

// stands in for the platform-specific ultrasonic sensor read
fn get_ultra_sonic_sensor_distance() -> f64 {
    0.42
}

fn main() -> Result<(), Box<dyn core::error::Error>> {
    use iceoryx2::prelude::*;

    let node = NodeBuilder::new().create::<ipc::Service>()?;

    // the distance publisher is reused from the publish-subscribe example
    let publisher = node
        .service_builder(&"distance_to_obstacle".try_into()?)
        .publish_subscribe::<Distance>()
        .open_or_create()?
        .publisher_builder()
        .create()?;

    // snippet:start service
    type KeyType = StaticString<50>;
    let service = node
        .service_builder(&"global_config".try_into()?)
        .blackboard_opener::<KeyType>()
        .open()?;
    // snippet:end service

    // snippet:start reader
    let reader = service.reader_builder().create()?;
    // snippet:end reader

    // snippet:start handle
    let update_rate_handle =
        reader.entry::<u32>(&"ultra_sonic_sensor_update_rate_in_ms".try_into()?)?;
    // snippet:end handle

    // snippet:start sensor-loop
    while node
        .wait(Duration::from_millis(*update_rate_handle.get() as u64))
        .is_ok()
    {
        let sample = publisher.loan_uninit()?;

        let sample = sample.write_payload(Distance {
            distance_in_meters: get_ultra_sonic_sensor_distance(),
            some_other_property: 42.0,
        });

        sample.send()?;
    }
    // snippet:end sensor-loop

    Ok(())
}
