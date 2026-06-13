#[allow(unused_variables)]
fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start
    use core::time::Duration;
    use iceoryx2::prelude::*;

    #[derive(Debug, Clone, Copy, ZeroCopySend)]
    #[repr(C)]
    struct WheelSpeed {
        timestamp_ns: u64,
        axle: u32,
        speed_mps: f32,
    }

    #[derive(Debug, Clone, Copy, ZeroCopySend)]
    #[repr(C)]
    struct ImuSample {
        timestamp_ns: u64,
        accel_mps2: f32,
        yaw_rate_rps: f32,
    }

    #[derive(Debug, Clone, Copy, ZeroCopySend)]
    #[repr(C)]
    struct ObjectTrack {
        timestamp_ns: u64,
        distance_m: f32,
        relative_speed_mps: f32,
    }

    const LOOKBACK_WINDOW: usize = 256;

    let node = NodeBuilder::new()
        .name(&"EventDataRecorder".try_into()?)
        .create::<ipc::Service>()?;

    let wheel_speed_service = node
        .service_builder(&"vehicle/wheel_speed".try_into()?)
        .publish_subscribe::<WheelSpeed>()
        .subscriber_max_buffer_size(LOOKBACK_WINDOW)
        .open_or_create()?;

    let imu_service = node
        .service_builder(&"vehicle/imu".try_into()?)
        .publish_subscribe::<ImuSample>()
        .subscriber_max_buffer_size(LOOKBACK_WINDOW)
        .open_or_create()?;

    let object_track_service = node
        .service_builder(&"vehicle/object_track".try_into()?)
        .publish_subscribe::<ObjectTrack>()
        .subscriber_max_buffer_size(LOOKBACK_WINDOW)
        .open_or_create()?;

    let trigger_service = node
        .service_builder(&"vehicle/incident_trigger".try_into()?)
        .event()
        .open_or_create()?;

    let wheel_speed = wheel_speed_service.subscriber_builder().create()?;
    let imu = imu_service.subscriber_builder().create()?;
    let object_track = object_track_service.subscriber_builder().create()?;
    let trigger = trigger_service.listener_builder().create()?;

    while node.wait(Duration::ZERO).is_ok() {
        if trigger.timed_wait(|_| {}, Duration::from_secs(1))? > 0 {
            while let Some(sample) = wheel_speed.receive()? {
                // append to upload bundle
            }
            while let Some(sample) = imu.receive()? {
                // append to upload bundle
            }
            while let Some(sample) = object_track.receive()? {
                // append to upload bundle
            }
            // ship the bundle upstream for offline scenario analysis
        }
    }
    // snippet:end

    Ok(())
}
