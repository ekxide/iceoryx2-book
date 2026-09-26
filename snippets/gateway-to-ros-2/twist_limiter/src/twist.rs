use iceoryx2::prelude::*;
use rosidl_runtime_rs::{Message, RmwMessage};

#[derive(Debug, Default, Clone)]
#[repr(transparent)]
pub struct Twist(pub ros_env::geometry_msgs::msg::rmw::Twist);

unsafe impl ZeroCopySend for Twist {
    unsafe fn type_name() -> &'static str {
        <<ros_env::geometry_msgs::msg::Twist as Message>::RmwMsg as RmwMessage>::TYPE_NAME
    }
}
