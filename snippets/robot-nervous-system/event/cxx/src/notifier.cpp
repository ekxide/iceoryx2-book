#include "iox2/iceoryx2.hpp"

namespace {
// stand-ins for Larry's sensor hardware
auto bump_sensor_was_activated() -> bool {
    return false;
}

auto battery_state() -> float {
    return 100.0F; // NOLINT
}

constexpr float battery_threshold = 20.0F; // NOLINT
} // namespace

auto main() -> int {
    using namespace iox2;

    // snippet:start node-and-service
    auto node = NodeBuilder().create<ServiceType::Ipc>().value();

    auto service =
        node.service_builder(ServiceName::create("system_health_events").value()).event().open_or_create().value();
    // snippet:end node-and-service

    // snippet:start notifier
    auto notifier = service.notifier_builder().create().value();
    // snippet:end notifier

    // snippet:start event-ids
    const auto wall_was_hit = EventId(0);
    const auto battery_is_low = EventId(1);
    // snippet:end event-ids

    // snippet:start notify-loop
    while (node.wait(iox2::bb::Duration::from_secs(1)).has_value()) {
        if (bump_sensor_was_activated()) {
            notifier.notify_with_custom_event_id(wall_was_hit).value();
        }

        if (battery_state() < battery_threshold) {
            notifier.notify_with_custom_event_id(battery_is_low).value();
        }
    }
    // snippet:end notify-loop

    return 0;
}
