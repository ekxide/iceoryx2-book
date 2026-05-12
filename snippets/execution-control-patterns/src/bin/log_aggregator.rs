#[allow(unused_variables)]
fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start
    use core::time::Duration;
    use iceoryx2::prelude::*;

    #[derive(Debug, Clone, Copy, ZeroCopySend)]
    #[repr(C)]
    struct LogEntry {
        timestamp_ns: u64,
        worker_id: u32,
        severity: u32,
    }

    let node = NodeBuilder::new()
        .name(&"LogAggregator".try_into()?)
        .create::<ipc::Service>()?;

    let publish_subscribe_service = node
        .service_builder(&"monitoring/logs".try_into()?)
        .publish_subscribe::<LogEntry>()
        .open_or_create()?;

    let event_service = node
        .service_builder(&"monitoring/logs".try_into()?)
        .event()
        .open_or_create()?;

    let subscriber = publish_subscribe_service.subscriber_builder().create()?;
    let listener = event_service.listener_builder().create()?;

    while node.wait(Duration::ZERO).is_ok() {
        if listener.timed_wait_one(Duration::from_secs(1))?.is_some() {
            while let Some(entry) = subscriber.receive()? {
                // write entry to sink
            }
        }
    }
    // snippet:end

    Ok(())
}
