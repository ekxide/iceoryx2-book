#include "iox2/bb/static_string.hpp"
#include "iox2/iceoryx2.hpp"

#include <iostream>

namespace {
// stand-ins for the user-facing configuration input
auto get_battery_threshold() -> iox2::bb::Optional<float> {
    return {};
}

auto get_update_rate() -> iox2::bb::Optional<uint32_t> {
    return {};
}
} // namespace

auto main() -> int {
    using namespace iox2;

    // snippet:start node
    auto node = NodeBuilder().create<ServiceType::Ipc>().value();
    // snippet:end node

    // snippet:start service
    using KeyType = bb::StaticString<50>;
    auto battery_key = bb::StaticString<50>::from_utf8("battery_threshold");
    auto us_sensor_key = bb::StaticString<50>::from_utf8("ultra_sonic_sensor_update_rate_in_ms");
    if (!battery_key.has_value() || !us_sensor_key.has_value()) {
        std::cerr << "Blackboard keys could not be created." << std::endl;
        return -1;
    }

    auto service = node.service_builder(ServiceName::create("global_config").value())
                       .blackboard_creator<KeyType>()
                       // low battery warning when load is below 25%
                       .template add<float>(battery_key.value(), 0.25F)
                       // default ultrasonic update rate = 100 ms
                       .template add<uint32_t>(us_sensor_key.value(), 100)
                       .create()
                       .value();
    // snippet:end service

    // snippet:start writer
    auto writer = service.writer_builder().create().value();
    // snippet:end writer

    // snippet:start entries
    auto battery_threshold_handle = writer.template entry<float>(battery_key.value()).value();
    auto update_rate_handle = writer.template entry<uint32_t>(us_sensor_key.value()).value();
    // snippet:end entries

    // snippet:start update-loop
    while (node.wait(iox2::bb::Duration::from_millis(100)).has_value()) {
        auto new_battery_threshold = get_battery_threshold();
        if (new_battery_threshold.has_value()) {
            // small value -> simple copy API
            battery_threshold_handle.update_with_copy(new_battery_threshold.value());
        }

        auto new_update_rate = get_update_rate();
        if (new_update_rate.has_value()) {
            update_rate_handle.update_with_copy(new_update_rate.value());
        }
    }
    // snippet:end update-loop

    return 0;
}
