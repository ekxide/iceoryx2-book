#include "iox2/iceoryx2.hpp"
#include "telemetry_data.hpp"

#include <utility>

// snippet:start
constexpr iox2::bb::Duration CYCLE_TIME = iox2::bb::Duration::from_secs(1);

auto main() -> int {
    using namespace iox2;
    set_log_level_from_env_or(LogLevel::Info);

    auto node = NodeBuilder().name(NodeName::create("Larry").value()).create<ServiceType::Ipc>().value();

    auto battery_publisher = node.service_builder(ServiceName::create("larry/battery").value())
                                 .publish_subscribe<BatteryState>()
                                 .open_or_create()
                                 .value()
                                 .publisher_builder()
                                 .create()
                                 .value();
    auto battery_notifier = node.service_builder(ServiceName::create("larry/battery").value())
                                .event()
                                .open_or_create()
                                .value()
                                .notifier_builder()
                                .create()
                                .value();

    auto position_publisher = node.service_builder(ServiceName::create("larry/position").value())
                                  .publish_subscribe<Position>()
                                  .open_or_create()
                                  .value()
                                  .publisher_builder()
                                  .create()
                                  .value();
    auto position_notifier = node.service_builder(ServiceName::create("larry/position").value())
                                 .event()
                                 .open_or_create()
                                 .value()
                                 .notifier_builder()
                                 .create()
                                 .value();

    while (node.wait(CYCLE_TIME).has_value()) {
        auto battery = battery_publisher.loan_uninit().value();
        auto initialized_battery = battery.write_payload(BatteryState { 87.0F }); // NOLINT
        send(std::move(initialized_battery)).has_value();
        battery_notifier.notify().has_value();

        auto position = position_publisher.loan_uninit().value();
        auto initialized_position = position.write_payload(Position { 1.0F, 2.0F }); // NOLINT
        send(std::move(initialized_position)).has_value();
        position_notifier.notify().has_value();
    }

    return 0;
}
// snippet:end
