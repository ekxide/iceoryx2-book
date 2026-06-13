use core::time::Duration;
use iceoryx2_bb_container::string::StaticString;

// stand-ins for the user-facing configuration input
fn get_battery_threshold() -> Option<f32> {
    None
}

fn get_update_rate() -> Option<u32> {
    None
}

fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start node
    use iceoryx2::prelude::*;

    let node = NodeBuilder::new().create::<ipc::Service>()?;
    // snippet:end node

    // snippet:start service
    type KeyType = StaticString<50>;
    let service = node
        .service_builder(&"global_config".try_into()?)
        .blackboard_creator::<KeyType>()
        // low battery warning when load is below 25%
        .add::<f32>(StaticString::try_from("battery_threshold")?, 0.25)
        // default ultrasonic update rate = 100 ms
        .add::<u32>(
            StaticString::try_from("ultra_sonic_sensor_update_rate_in_ms")?,
            100,
        )
        .create()?;
    // snippet:end service

    // snippet:start writer
    let writer = service.writer_builder().create()?;
    // snippet:end writer

    // snippet:start entries
    let battery_threshold_handle = writer.entry::<f32>(&"battery_threshold".try_into()?)?;
    let update_rate_handle =
        writer.entry::<u32>(&"ultra_sonic_sensor_update_rate_in_ms".try_into()?)?;
    // snippet:end entries

    // snippet:start update-loop
    while node.wait(Duration::from_millis(100)).is_ok() {
        if let Some(new_battery_threshold) = get_battery_threshold() {
            // small value -> simple copy API
            battery_threshold_handle.update_with_copy(new_battery_threshold);
        }

        if let Some(new_update_rate) = get_update_rate() {
            update_rate_handle.update_with_copy(new_update_rate);
        }
    }
    // snippet:end update-loop

    Ok(())
}
