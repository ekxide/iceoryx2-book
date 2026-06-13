#include "distance.hpp"
#include "iox2/iceoryx2.hpp"

#include <utility>

namespace {
// stands in for the platform-specific ultrasonic sensor read
auto get_ultra_sonic_sensor_distance() -> double {
    return 0.42; // NOLINT
}

constexpr double distance_threshold = 1.0; // NOLINT
} // namespace

auto main() -> int {
    using namespace iox2;

    // snippet:start pubsub-service
    auto node = NodeBuilder().create<ServiceType::Ipc>().value();

    auto pubsub_service = node.service_builder(ServiceName::create("distance_to_obstacle").value())
                              .publish_subscribe<Distance>()
                              .subscriber_max_buffer_size(3)
                              .history_size(3)
                              .subscriber_max_borrowed_samples(3)
                              .open_or_create()
                              .value();
    // snippet:end pubsub-service

    // snippet:start event-service
    auto ultra_sonic_service_dead = EventId(10);
    auto event_service = node.service_builder(ServiceName::create("distance_to_obstacle").value())
                             .event()
                             .notifier_dead_event(ultra_sonic_service_dead)
                             .open_or_create()
                             .value();
    // snippet:end event-service

    // snippet:start ports
    auto publisher = pubsub_service.publisher_builder().create().value();
    auto notifier = event_service.notifier_builder().create().value();

    auto obstacle_too_close = EventId(5);
    // snippet:end ports

    // snippet:start publish-loop
    while (node.wait(iox2::bb::Duration::from_millis(100)).has_value()) {
        auto sample = publisher.loan_uninit().value();

        auto distance = get_ultra_sonic_sensor_distance();
        auto initialized_sample = sample.write_payload(Distance { distance, 42.0 }); // NOLINT

        send(std::move(initialized_sample)).value();

        if (distance < distance_threshold) {
            notifier.notify_with_custom_event_id(obstacle_too_close).value();
        }
    }
    // snippet:end publish-loop

    return 0;
}
