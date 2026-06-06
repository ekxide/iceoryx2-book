// snippet:start
use iceoryx2::prelude::*;

#[derive(Debug, Clone, Copy, ZeroCopySend)]
// shared type name; must match across languages
#[type_name("BatteryState")]
#[repr(C)]
pub struct BatteryState {
    pub charge_percent: f32,
}

#[derive(Debug, Clone, Copy, ZeroCopySend)]
// shared type name; must match across languages
#[type_name("Position")]
#[repr(C)]
pub struct Position {
    pub x: f32,
    pub y: f32,
}
// snippet:end
