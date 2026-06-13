#include "autopilot.hpp"
#include "iox2/iceoryx2.hpp"

#include <utility>

namespace {
// stand-ins for Larry's autopilot
template <typename T>
void drive_to_position(const T& /* request */) {
}

void stop_driving() {
}

auto arrived_at_destination() -> bool {
    return false;
}

auto get_current_state() -> State {
    return State { { 0.0F, 0.0F }, { 0.0F, 0.0F } };
}
} // namespace

auto main() -> int {
    using namespace iox2;

    // snippet:start setup
    auto node = NodeBuilder().create<ServiceType::Ipc>().value();

    auto service = node.service_builder(ServiceName::create("autopilot").value())
                       .request_response<Position, State>()
                       .open_or_create()
                       .value();

    auto server = service.server_builder().create().value();
    // snippet:end setup

    // snippet:start serve-loop
    auto active_request = server.receive().value();
    while (node.wait(iox2::bb::Duration::from_millis(100)).has_value()) {
        if (!active_request.has_value()) {
            active_request = server.receive().value();
            if (active_request.has_value()) {
                drive_to_position(active_request.value());
            }
        }

        if (active_request.has_value()) {
            if (!active_request->is_connected()) {
                stop_driving();
                active_request.reset();
            } else if (arrived_at_destination()) {
                active_request.reset();
            } else {
                auto response = active_request->loan_uninit().value();
                auto initialized_response = response.write_payload(get_current_state());
                send(std::move(initialized_response)).value();
            }
        }
    }
    // snippet:end serve-loop

    return 0;
}
