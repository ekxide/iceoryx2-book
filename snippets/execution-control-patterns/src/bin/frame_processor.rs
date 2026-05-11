#[allow(unused_variables)]
fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start
    use core::time::Duration;
    use iceoryx2::prelude::*;

    #[derive(Debug, Clone, Copy, ZeroCopySend)]
    #[repr(C)]
    struct CameraFrame {
        frame_id: u64,
        width: u32,
        height: u32,
        timestamp_ns: u64,
    }

    let node = NodeBuilder::new()
        .name(&"FrameProcessor".try_into()?)
        .create::<ipc::Service>()?;

    let frames_service = node
        .service_builder(&"camera/frames".try_into()?)
        .publish_subscribe::<CameraFrame>()
        .open_or_create()?;

    let signal_service = node
        .service_builder(&"camera/frames".try_into()?)
        .event()
        .open_or_create()?;

    let subscriber = frames_service.subscriber_builder().create()?;
    let listener = signal_service.listener_builder().create()?;

    while node.wait(Duration::ZERO).is_ok() {
        if listener.timed_wait_one(Duration::from_secs(1))?.is_some() {
            while let Some(frame) = subscriber.receive()? {
                // process frame
            }
        }
    }
    // snippet:end

    Ok(())
}
