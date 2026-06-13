#include "iox2/iceoryx2.hpp"

namespace {
// stand-ins for the health-monitor's reactions
void activate_battery_warning_light() {
}

void go_into_parking_position() {
}
} // namespace

auto main() -> int {
    using namespace iox2;

    // snippet:start node-and-service
    auto node = NodeBuilder().create<ServiceType::Ipc>().value();

    auto service =
        node.service_builder(ServiceName::create("system_health_events").value()).event().open_or_create().value();
    // snippet:end node-and-service

    // snippet:start listener
    auto listener = service.listener_builder().create().value();
    // snippet:end listener

    const auto wall_was_hit = EventId(0);
    const auto battery_is_low = EventId(1);

    // snippet:start wait-loop
    while (listener
               .blocking_wait([&](auto event) {
                   if (event.id() == battery_is_low) {
                       activate_battery_warning_light();
                   }

                   if (event.id() == wall_was_hit) {
                       go_into_parking_position();
                   }
               })
               .has_value()) {
    }
    // snippet:end wait-loop

    return 0;
}
