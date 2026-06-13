#include "distance.hpp"
#include "iox2/bb/static_string.hpp"
#include "iox2/iceoryx2.hpp"

#include <iostream>
#include <utility>

namespace {
// stands in for the platform-specific ultrasonic sensor read
auto get_ultra_sonic_sensor_distance() -> double {
    return 0.42; // NOLINT
}
} // namespace

auto main() -> int {
    using namespace iox2;

    auto node = NodeBuilder().create<ServiceType::Ipc>().value();

    // the distance publisher is reused from the publish-subscribe example
    auto publisher = node.service_builder(ServiceName::create("distance_to_obstacle").value())
                         .publish_subscribe<Distance>()
                         .open_or_create()
                         .value()
                         .publisher_builder()
                         .create()
                         .value();

    // snippet:start service
    using KeyType = bb::StaticString<50>;
    auto service =
        node.service_builder(ServiceName::create("global_config").value()).blackboard_opener<KeyType>().open().value();
    // snippet:end service

    // snippet:start reader
    auto reader = service.reader_builder().create().value();
    // snippet:end reader

    // snippet:start handle
    auto us_sensor_key = bb::StaticString<50>::from_utf8("ultra_sonic_sensor_update_rate_in_ms").value();
    auto update_rate_handle = reader.template entry<uint32_t>(us_sensor_key).value();
    // snippet:end handle

    // snippet:start sensor-loop
    while (node.wait(iox2::bb::Duration::from_millis(*update_rate_handle.get())).has_value()) {
        auto sample = publisher.loan_uninit().value();

        auto initialized_sample = sample.write_payload(Distance { get_ultra_sonic_sensor_distance(), 42.0 }); // NOLINT

        send(std::move(initialized_sample)).value();
    }
    // snippet:end sensor-loop

    return 0;
}
