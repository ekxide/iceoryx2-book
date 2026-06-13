use core::time::Duration;
use event_driven_communication::distance::Distance;

// stands in for the emergency-brake reactions
fn go_into_parking_position() {}

fn perform_break<T>(_last_samples: Vec<T>) {}

fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start setup
    use iceoryx2::prelude::*;

    let node = NodeBuilder::new().create::<ipc::Service>()?;

    let pubsub_service = node
        .service_builder(&"distance_to_obstacle".try_into()?)
        .publish_subscribe::<Distance>()
        .subscriber_max_buffer_size(3)
        .history_size(3)
        .subscriber_max_borrowed_samples(3)
        .open_or_create()?;

    let ultra_sonic_service_dead = EventId::new(10);
    let event_service = node
        .service_builder(&"distance_to_obstacle".try_into()?)
        .event()
        .notifier_dead_event(ultra_sonic_service_dead)
        .open_or_create()?;

    let subscriber = pubsub_service.subscriber_builder().create()?;
    let listener = event_service.listener_builder().create()?;
    // snippet:end setup

    let obstacle_too_close = EventId::new(5);

    // snippet:start wait-loop
    while node.wait(Duration::ZERO).is_ok() {
        let mut sensor_is_dead = false;
        let mut obstacle_detected = false;

        listener.blocking_wait(|event| {
            if event.id == ultra_sonic_service_dead {
                sensor_is_dead = true;
            }
            if event.id == obstacle_too_close {
                obstacle_detected = true;
            }
        })?;

        if sensor_is_dead {
            go_into_parking_position();
        }

        if obstacle_detected {
            let mut last_samples = vec![];
            while let Some(sample) = subscriber.receive()? {
                last_samples.push(sample);
                if last_samples.len() == 3 {
                    break;
                }
            }
            perform_break(last_samples);
        }
    }
    // snippet:end wait-loop

    Ok(())
}
