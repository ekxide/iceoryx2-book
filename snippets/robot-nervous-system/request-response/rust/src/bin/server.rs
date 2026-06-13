use core::time::Duration;
use request_response::autopilot::{Position, State};

// stand-ins for Larry's autopilot
fn drive_to_position<T>(_request: &T) {}

fn stop_driving() {}

fn arrived_at_destination() -> bool {
    false
}

fn get_current_state() -> State {
    State {
        position: [0.0, 0.0],
        speed: [0.0, 0.0],
    }
}

fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start setup
    use iceoryx2::prelude::*;

    let node = NodeBuilder::new().create::<ipc::Service>()?;
    let service = node
        .service_builder(&"autopilot".try_into()?)
        .request_response::<Position, State>()
        .open_or_create()?;

    let server = service.server_builder().create()?;
    // snippet:end setup

    // snippet:start serve-loop
    let mut active_request = None;
    while node.wait(Duration::from_millis(100)).is_ok() {
        if active_request.is_none() {
            if let Some(new_request) = server.receive()? {
                drive_to_position(&new_request);
                active_request = Some(new_request);
            }
        }

        if let Some(current_request) = active_request.take() {
            if !current_request.is_connected() {
                stop_driving();
            } else if !arrived_at_destination() {
                let response = current_request.loan_uninit()?;
                let response = response.write_payload(get_current_state());
                response.send()?;
                active_request = Some(current_request);
            }
        }
    }
    // snippet:end serve-loop

    Ok(())
}
