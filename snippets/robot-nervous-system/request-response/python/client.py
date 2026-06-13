"""Autopilot client."""

from autopilot import Position, State


def show_larry_position_in_app(state) -> None:
    """Stand in for the app's position display."""


def is_at_destination(state) -> bool:
    """Stand in for the destination check."""
    return False


def show_larry_arrived_popup_in_app() -> None:
    """Stand in for the arrival popup."""


def show_larry_encountered_obstacle_in_app() -> None:
    """Stand in for the obstacle popup."""


def user_has_pressed_stop_button() -> bool:
    """Stand in for the stop button."""
    return False


# snippet:start node
import iceoryx2 as iox2

node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)
# snippet:end node

# snippet:start service
service = (
    node.service_builder(iox2.ServiceName.new("autopilot"))
    .request_response(Position, State)
    .open_or_create()
)
# snippet:end service

# snippet:start client
client = service.client_builder().create()
# snippet:end client

# snippet:start send-request
request = client.loan_uninit()
position = Position()
position.position[0] = 123.456
position.position[1] = 789.1
request = request.write_payload(position)
pending_response = request.send()
# snippet:end send-request

# snippet:start event-loop
try:
    while True:
        node.wait(iox2.Duration.from_millis(100))

        while True:
            response = pending_response.receive()
            if response is None:
                break

            show_larry_position_in_app(response)

            if not pending_response.is_connected():
                if is_at_destination(response):
                    show_larry_arrived_popup_in_app()
                else:
                    show_larry_encountered_obstacle_in_app()

        if user_has_pressed_stop_button():
            break

except iox2.NodeWaitFailure:
    print("exit")
# snippet:end event-loop
