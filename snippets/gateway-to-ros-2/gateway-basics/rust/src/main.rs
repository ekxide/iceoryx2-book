use iceoryx2::prelude::*;

// snippet:start payload
#[derive(Debug, ZeroCopySend)]
#[type_name("std_msgs/msg/Float64")]
#[repr(C)]
pub struct Payload {
    pub data: f64,
}
// snippet:end payload

fn main() -> Result<(), Box<dyn core::error::Error>> {
    let node = NodeBuilder::new().create::<ipc::Service>()?;

    // snippet:start user-header
    use iceoryx2_integrations_ros2_interop::RosHeader;

    let service = node
        .service_builder(&"MyService".try_into()?)
        .publish_subscribe::<Payload>()
        .user_header::<RosHeader>()
        .open_or_create()?;
    // snippet:end user-header

    let _ = service;
    let _ = Payload { data: 0.0 }.data;
    Ok(())
}
