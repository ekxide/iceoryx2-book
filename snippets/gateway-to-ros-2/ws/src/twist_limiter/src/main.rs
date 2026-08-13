// snippet:start payload
use iceoryx2::prelude::*;
use rosidl_runtime_rs::{Message, RmwMessage};

#[derive(Debug, Default, Clone)]
#[repr(transparent)]
pub struct Twist(pub geometry_msgs::msg::rmw::Twist);

unsafe impl ZeroCopySend for Twist {
    unsafe fn type_name() -> &'static str {
        <<geometry_msgs::msg::Twist as Message>::RmwMsg as RmwMessage>::TYPE_NAME
    }
}
// snippet:end payload

// snippet:start limiter
use core::time::Duration;

use iceoryx2_integrations_ros2_interop::RosHeader;

const CYCLE_TIME: Duration = Duration::from_millis(100);
const MAX_VELOCITY_M_PER_S: f64 = 1.0;

fn limit(twist: &Twist) -> Twist {
    let mut limited = twist.clone();
    limited.0.linear.x = limited
        .0
        .linear
        .x
        .clamp(-MAX_VELOCITY_M_PER_S, MAX_VELOCITY_M_PER_S);
    limited
}

fn main() -> Result<(), Box<dyn core::error::Error>> {
    set_log_level_from_env_or(LogLevel::Info);

    let node = NodeBuilder::new().create::<ipc::Service>()?;

    let cmd_vel = node
        .service_builder(&"CmdVel".try_into()?)
        .publish_subscribe::<Twist>()
        // IMPORTANT: Must use this user header if crossing ROS 2 boundary.
        .user_header::<RosHeader>()
        .open_or_create()?;
    let subscriber = cmd_vel.subscriber_builder().create()?;

    let cmd_vel_limited = node
        .service_builder(&"CmdVelLimited".try_into()?)
        .publish_subscribe::<Twist>()
        // IMPORTANT: Must use this user header if crossing ROS 2 boundary.
        .user_header::<RosHeader>()
        .open_or_create()?;
    let publisher = cmd_vel_limited.publisher_builder().create()?;

    while node.wait(CYCLE_TIME).is_ok() {
        while let Some(sample) = subscriber.receive()? {
            let limited = limit(sample.payload());
            publisher.loan_uninit()?.write_payload(limited).send()?;

            coutln!(
                "limited cmd_vel (sequence {})",
                sample.user_header().sequence_number
            );
        }
    }

    Ok(())
}
// snippet:end limiter
