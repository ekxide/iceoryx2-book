use iceoryx2::prelude::*;

#[derive(Debug, ZeroCopySend)]
#[repr(C)]
pub struct Distance {
    pub distance_in_meters: f64,
    pub some_other_property: f32,
}
