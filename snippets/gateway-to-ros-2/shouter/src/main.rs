// snippet:start payload
use iceoryx2::prelude::*;
use rosidl_runtime_rs::{Message, RmwMessage};

#[derive(Debug, Clone, Copy)]
#[repr(transparent)]
pub struct StringByte(pub u8);

unsafe impl ZeroCopySend for StringByte {
    unsafe fn type_name() -> &'static str {
        <<ros_env::std_msgs::msg::String as Message>::RmwMsg as RmwMessage>::TYPE_NAME
    }
}
// snippet:end payload

// snippet:start shouter
use core::time::Duration;

use cdr::{CdrLe, Infinite};

const CYCLE_TIME: Duration = Duration::from_millis(100);
const INITIAL_MAX_PAYLOAD_SIZE: usize = 64;

fn main() -> Result<(), Box<dyn core::error::Error>> {
    set_log_level_from_env_or(LogLevel::Info);

    let node = NodeBuilder::new().create::<ipc::Service>()?;

    let chatter = node
        .service_builder(&"Chatter".try_into()?)
        .publish_subscribe::<[StringByte]>()
        .open_or_create()?;
    let subscriber = chatter.subscriber_builder().create()?;

    let shouter = node
        .service_builder(&"Shouter".try_into()?)
        .publish_subscribe::<[StringByte]>()
        .open_or_create()?;
    let publisher = shouter
        .publisher_builder()
        .initial_max_slice_len(INITIAL_MAX_PAYLOAD_SIZE)
        .allocation_strategy(AllocationStrategy::PowerOfTwo)
        .create()?;

    while node.wait(CYCLE_TIME).is_ok() {
        while let Some(sample) = subscriber.receive()? {
            let bytes: Vec<u8> = sample.payload().iter().map(|byte| byte.0).collect();

            // The application must deserialize the CDR bytes itself
            let mut message: ros_env::std_msgs::msg::String = cdr::deserialize(&bytes)?;
            message.data = message.data.to_uppercase();

            // And then serialize the outgoing data back to CDR
            let payload = cdr::serialize::<_, _, CdrLe>(&message, Infinite)?;
            let upper_sample = publisher.loan_slice_uninit(payload.len())?;
            let upper_sample = upper_sample.write_from_fn(|index| StringByte(payload[index]));
            upper_sample.send()?;

            coutln!("shouted \"{}\" ({} bytes)", message.data, payload.len());
        }
    }

    Ok(())
}
// snippet:end shouter
