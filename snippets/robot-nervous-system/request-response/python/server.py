"""Autopilot server."""

from autopilot import Position, State


def drive_to_position(request) -> None:
    """Stand in for Larry's autopilot."""


def stop_driving() -> None:
    """Stand in for stopping Larry."""


def arrived_at_destination() -> bool:
    """Stand in for the arrival check."""
    return False


def get_current_state() -> State:
    """Stand in for sampling Larry's current state."""
    return State()


# snippet:start setup
import iceoryx2 as iox2

node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)

service = (
    node.service_builder(iox2.ServiceName.new("autopilot"))
    .request_response(Position, State)
    .open_or_create()
)

server = service.server_builder().create()
# snippet:end setup

# snippet:start serve-loop
active_request = None
try:
    while True:
        node.wait(iox2.Duration.from_millis(100))

        if active_request is None:
            active_request = server.receive()
            if active_request is not None:
                drive_to_position(active_request)

        if active_request is not None:
            if not active_request.is_connected():
                stop_driving()
                active_request = None
            elif arrived_at_destination():
                active_request = None
            else:
                response = active_request.loan_uninit()
                response = response.write_payload(get_current_state())
                response.send()

except iox2.NodeWaitFailure:
    print("exit")
# snippet:end serve-loop
