#include "distance.hpp"
#include "iox2/iceoryx2.hpp"

#include <utility>
#include <vector>

namespace {
// stands in for the emergency-brake reactions
void go_into_parking_position() {
}

template <typename T>
void perform_break(const std::vector<T>& /* last_samples */) {
}
} // namespace

auto main() -> int {
    using namespace iox2;

    // snippet:start setup
    auto node = NodeBuilder().create<ServiceType::Ipc>().value();

    auto pubsub_service = node.service_builder(ServiceName::create("distance_to_obstacle").value())
                              .publish_subscribe<Distance>()
                              .subscriber_max_buffer_size(3)
                              .history_size(3)
                              .subscriber_max_borrowed_samples(3)
                              .open_or_create()
                              .value();

    auto ultra_sonic_service_dead = EventId(10);
    auto event_service = node.service_builder(ServiceName::create("distance_to_obstacle").value())
                             .event()
                             .notifier_dead_event(ultra_sonic_service_dead)
                             .open_or_create()
                             .value();

    auto subscriber = pubsub_service.subscriber_builder().create().value();
    auto listener = event_service.listener_builder().create().value();
    // snippet:end setup

    auto obstacle_too_close = EventId(5);

    // snippet:start wait-loop
    while (node.wait(iox2::bb::Duration::from_secs(0)).has_value()) {
        auto sensor_is_dead = false;
        auto obstacle_detected = false;

        listener
            .blocking_wait([&](auto event) {
                if (event.id() == ultra_sonic_service_dead) {
                    sensor_is_dead = true;
                }
                if (event.id() == obstacle_too_close) {
                    obstacle_detected = true;
                }
            })
            .has_value();

        if (sensor_is_dead) {
            go_into_parking_position();
        }

        if (obstacle_detected) {
            std::vector<Sample<ServiceType::Ipc, Distance, void>> last_samples;
            auto sample = subscriber.receive().value();
            while (sample.has_value()) {
                last_samples.push_back(std::move(sample.value()));
                if (last_samples.size() == 3) {
                    break;
                }
                sample = subscriber.receive().value();
            }
            perform_break(last_samples);
        }
    }
    // snippet:end wait-loop

    return 0;
}
