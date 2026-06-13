#include "autopilot.hpp"
#include "iox2/iceoryx2.hpp"

#include <utility>

namespace {
// stand-ins for the user-facing app
template <typename T>
void show_larry_position_in_app(const T& /* state */) {
}

template <typename T>
auto is_at_destination(const T& /* state */) -> bool {
    return false;
}

void show_larry_arrived_popup_in_app() {
}

void show_larry_encountered_obstacle_in_app() {
}

auto user_has_pressed_stop_button() -> bool {
    return false;
}
} // namespace

auto main() -> int {
    using namespace iox2;

    // snippet:start node
    auto node = NodeBuilder().create<ServiceType::Ipc>().value();
    // snippet:end node

    // snippet:start service
    auto service = node.service_builder(ServiceName::create("autopilot").value())
                       .request_response<Position, State>()
                       .open_or_create()
                       .value();
    // snippet:end service

    // snippet:start client
    auto client = service.client_builder().create().value();
    // snippet:end client

    // snippet:start send-request
    auto request = client.loan_uninit().value();
    auto initialized_request = request.write_payload(Position { { 123.456F, 789.1F } }); // NOLINT

    auto pending_response = send(std::move(initialized_request)).value();
    // snippet:end send-request

    // snippet:start event-loop
    while (node.wait(iox2::bb::Duration::from_millis(100)).has_value()) {
        auto response = pending_response.receive().value();
        while (response.has_value()) {
            show_larry_position_in_app(response.value());

            if (!pending_response.is_connected()) {
                if (is_at_destination(response.value())) {
                    show_larry_arrived_popup_in_app();
                } else {
                    show_larry_encountered_obstacle_in_app();
                }
            }

            response = pending_response.receive().value();
        }

        if (user_has_pressed_stop_button()) {
            // dropping the pending response cancels the stream
            break;
        }
    }
    // snippet:end event-loop

    return 0;
}
