use core::time::Duration;
use request_response::autopilot::{Position, State};

// stand-ins for the user-facing app
fn show_larry_position_in_app<T>(_state: &T) {}

fn is_at_destination<T>(_state: &T) -> bool {
    false
}

fn show_larry_arrived_popup_in_app() {}

fn show_larry_encountered_obstacle_in_app() {}

fn user_has_pressed_stop_button() -> bool {
    false
}

fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start node
    use iceoryx2::prelude::*;

    let node = NodeBuilder::new().create::<ipc::Service>()?;
    // snippet:end node

    // snippet:start service
    let service = node
        .service_builder(&"autopilot".try_into()?)
        .request_response::<Position, State>()
        .open_or_create()?;
    // snippet:end service

    // snippet:start client
    let client = service.client_builder().create()?;
    // snippet:end client

    // snippet:start send-request
    let request = client.loan_uninit()?;
    let request = request.write_payload(Position {
        position: [123.456, 789.1],
    });
    let pending_response = request.send()?;
    // snippet:end send-request

    // snippet:start event-loop
    while node.wait(Duration::from_millis(100)).is_ok() {
        while let Some(response) = pending_response.receive()? {
            show_larry_position_in_app(&response);

            if !pending_response.is_connected() {
                if is_at_destination(&response) {
                    show_larry_arrived_popup_in_app();
                } else {
                    show_larry_encountered_obstacle_in_app();
                }
            }
        }

        if user_has_pressed_stop_button() {
            // dropping the pending response cancels the stream
            drop(pending_response);
            break;
        }
    }
    // snippet:end event-loop

    Ok(())
}
