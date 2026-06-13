use core::time::Duration;
use publish_subscribe::distance::Distance;

fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start node-and-service
    use iceoryx2::prelude::*;

    let node = NodeBuilder::new().create::<ipc::Service>()?;

    let service = node
        .service_builder(&"distance_to_obstacle".try_into()?)
        .publish_subscribe::<Distance>()
        .open_or_create()?;
    // snippet:end node-and-service

    // snippet:start subscriber
    let subscriber = service.subscriber_builder().create()?;
    // snippet:end subscriber

    // snippet:start receive-loop
    while node.wait(Duration::from_millis(100)).is_ok() {
        while let Some(sample) = subscriber.receive()? {
            println!("received distance {:?}", sample.payload());
        }
    }
    // snippet:end receive-loop

    Ok(())
}
