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
