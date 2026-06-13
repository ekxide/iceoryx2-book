use core::time::Duration;
use event_driven_communication::distance::Distance;

// stands in for the platform-specific ultrasonic sensor read
fn get_ultra_sonic_sensor_distance() -> f64 {
    0.42
}

fn main() -> Result<(), Box<dyn core::error::Error>> {
    let distance_threshold = 1.0;

    // snippet:start pubsub-service
    use iceoryx2::prelude::*;

    let node = NodeBuilder::new().create::<ipc::Service>()?;

    let pubsub_service = node
        .service_builder(&"distance_to_obstacle".try_into()?)
        .publish_subscribe::<Distance>()
        .subscriber_max_buffer_size(3)
        .history_size(3)
        .subscriber_max_borrowed_samples(3)
        .open_or_create()?;
    // snippet:end pubsub-service

    // snippet:start event-service
    let ultra_sonic_service_dead = EventId::new(10);
    let event_service = node
        .service_builder(&"distance_to_obstacle".try_into()?)
        .event()
        .notifier_dead_event(ultra_sonic_service_dead)
        .open_or_create()?;
    // snippet:end event-service

    // snippet:start ports
    let publisher = pubsub_service.publisher_builder().create()?;
    let notifier = event_service.notifier_builder().create()?;

    let obstacle_too_close = EventId::new(5);
    // snippet:end ports

    // snippet:start publish-loop
    while node.wait(Duration::from_millis(100)).is_ok() {
        let sample = publisher.loan_uninit()?;

        let distance = get_ultra_sonic_sensor_distance();
        let sample = sample.write_payload(Distance {
            distance_in_meters: distance,
            some_other_property: 42.0,
        });

        sample.send()?;

        if distance < distance_threshold {
            notifier.notify_with_custom_event_id(obstacle_too_close)?;
        }
    }
    // snippet:end publish-loop

    Ok(())
}
