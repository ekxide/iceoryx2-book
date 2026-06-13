#include "distance.hpp"
#include "iox2/iceoryx2.hpp"

#include <utility>

namespace {
// stands in for the platform-specific ultrasonic sensor read
auto get_ultra_sonic_sensor_distance() -> double {
    return 0.42; // NOLINT
}
} // namespace

auto main() -> int {
    // snippet:start node
    using namespace iox2;

    auto node = NodeBuilder().name(NodeName::create("UltraSonicSensor").value()).create<ServiceType::Ipc>().value();
    // snippet:end node

    // snippet:start service
    auto service = node.service_builder(ServiceName::create("distance_to_obstacle").value())
                       .publish_subscribe<Distance>()
                       .open_or_create()
                       .value();
    // snippet:end service

    // snippet:start publisher
    auto publisher = service.publisher_builder().create().value();
    // snippet:end publisher

    // snippet:start publish-loop
    while (node.wait(iox2::bb::Duration::from_millis(100)).has_value()) {
        auto sample = publisher.loan_uninit().value();

        auto initialized_sample = sample.write_payload(Distance { get_ultra_sonic_sensor_distance(), 42.0 }); // NOLINT

        send(std::move(initialized_sample)).value();
    }
    // snippet:end publish-loop

    return 0;
}
