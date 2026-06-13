use iceoryx2::prelude::*;

// snippet:start request-type
#[derive(Debug, ZeroCopySend)]
#[repr(C)]
pub struct Position {
    pub position: [f32; 2],
}
// snippet:end request-type

// snippet:start response-type
#[derive(Debug, ZeroCopySend)]
#[repr(C)]
pub struct State {
    pub position: [f32; 2],
    pub speed: [f32; 2],
}
// snippet:end response-type
