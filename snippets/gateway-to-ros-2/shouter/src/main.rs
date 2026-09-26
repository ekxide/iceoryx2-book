// snippet:start payload
use iceoryx2::prelude::*;
use rosidl_runtime_rs::{Message, RmwMessage};

#[derive(Debug, Default, Clone, Copy)]
#[repr(transparent)]
pub struct StringByte(pub u8);

unsafe impl ZeroCopySend for StringByte {
    unsafe fn type_name() -> &'static str {
        <<ros_env::std_msgs::msg::String as Message>::RmwMsg as RmwMessage>::TYPE_NAME
    }
}

// Viewing a payload as bytes relies on StringByte having the layout of a u8.
const _: () = assert!(size_of::<StringByte>() == 1 && align_of::<StringByte>() == 1);

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
        while let Some(chatter_sample) = subscriber.receive()? {
            // The application deserializes the CDR bytes itself, directly from
            // shared memory
            let chatter_payload = chatter_sample.payload();

            // SAFETY: StringByte is #[repr(transparent)] over u8, so the bytes have
            // the same length, layout and valid values.
            let chatter_bytes = unsafe {
                core::slice::from_raw_parts(
                    chatter_payload.as_ptr().cast::<u8>(),
                    chatter_payload.len(),
                )
            };

            let mut message: ros_env::std_msgs::msg::String = cdr::deserialize(chatter_bytes)?;
            message.data.make_ascii_uppercase();

            // And then serializes the outgoing message back to CDR, directly into
            // a sample loaned for its exact size
            let size = cdr::calc_serialized_size(&message) as usize;
            let mut shouter_sample = publisher.loan_slice(size)?;
            let shouter_payload = shouter_sample.payload_mut();

            // SAFETY: StringByte is #[repr(transparent)] over u8, so the bytes have
            // the same length, layout and valid values.
            let shouter_bytes = unsafe {
                core::slice::from_raw_parts_mut(
                    shouter_payload.as_mut_ptr().cast::<u8>(),
                    shouter_payload.len(),
                )
            };

            cdr::serialize_into::<_, _, _, CdrLe>(shouter_bytes, &message, Infinite)?;
            shouter_sample.send()?;

            coutln!("shouted \"{}\" ({} bytes)", message.data, size);
        }
    }

    Ok(())
}
// snippet:end shouter
