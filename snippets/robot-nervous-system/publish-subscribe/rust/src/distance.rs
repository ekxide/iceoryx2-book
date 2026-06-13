// snippet:start payload
use iceoryx2::prelude::*;

#[derive(Debug, ZeroCopySend)] // every payload must implement ZeroCopySend
#[repr(C)] // ensures a consistent and well-defined layout
pub struct Distance {
    pub distance_in_meters: f64,
    pub some_other_property: f32,
}
// snippet:end payload
