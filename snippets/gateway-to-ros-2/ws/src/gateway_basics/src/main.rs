use iceoryx2::prelude::*;

// snippet:start payload
#[derive(Debug, ZeroCopySend)]
#[type_name("std_msgs/msg/Float64")]
#[repr(C)]
pub struct Payload {
    pub data: f64,
}
// snippet:end payload

// snippet:start wrapped-payload
use rosidl_runtime_rs::{Message, RmwMessage};

#[derive(Debug, Default, Clone)]
#[repr(transparent)]
pub struct Float64(pub std_msgs::msg::rmw::Float64);

unsafe impl ZeroCopySend for Float64 {
    unsafe fn type_name() -> &'static str {
        <<std_msgs::msg::Float64 as Message>::RmwMsg as RmwMessage>::TYPE_NAME
    }
}
// snippet:end wrapped-payload

fn main() -> Result<(), Box<dyn core::error::Error>> {
    let node = NodeBuilder::new().create::<ipc::Service>()?;

    // snippet:start user-header
    use iceoryx2_integrations_ros2_interop::RosHeader;

    let service = node
        .service_builder(&"MyService".try_into()?)
        .publish_subscribe::<Payload>()
        // IMPORTANT: Must use this user header if crossing ROS 2 boundary.
        .user_header::<RosHeader>()
        .open_or_create()?;
    // snippet:end user-header

    let wrapped = node
        .service_builder(&"MyWrappedService".try_into()?)
        .publish_subscribe::<Float64>()
        .open_or_create()?;

    let _ = service;
    let _ = wrapped;
    let _ = Payload { data: 0.0 }.data;
    Ok(())
}
