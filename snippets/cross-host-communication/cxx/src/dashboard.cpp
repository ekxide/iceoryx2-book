#include "iox2/iceoryx2.hpp"
#include "telemetry_data.hpp"

#include <iostream>

// snippet:start
auto main() -> int {
    using namespace iox2;
    set_log_level_from_env_or(LogLevel::Info);

    auto node = NodeBuilder().name(NodeName::create("Dashboard").value()).create<ServiceType::Ipc>().value();

    auto battery_subscriber = node.service_builder(ServiceName::create("larry/battery").value())
                                  .publish_subscribe<BatteryState>()
                                  .open_or_create()
                                  .value()
                                  .subscriber_builder()
                                  .create()
                                  .value();
    auto battery_listener = node.service_builder(ServiceName::create("larry/battery").value())
                                .event()
                                .open_or_create()
                                .value()
                                .listener_builder()
                                .create()
                                .value();

    auto position_subscriber = node.service_builder(ServiceName::create("larry/position").value())
                                   .publish_subscribe<Position>()
                                   .open_or_create()
                                   .value()
                                   .subscriber_builder()
                                   .create()
                                   .value();
    auto position_listener = node.service_builder(ServiceName::create("larry/position").value())
                                 .event()
                                 .open_or_create()
                                 .value()
                                 .listener_builder()
                                 .create()
                                 .value();

    auto waitset = WaitSetBuilder().create<ServiceType::Ipc>().value();
    auto battery_guard = waitset.attach_notification(battery_listener).value();
    auto position_guard = waitset.attach_notification(position_listener).value();

    auto on_event = [&](WaitSetAttachmentId<ServiceType::Ipc> attachment_id) -> CallbackProgression {
        if (attachment_id.has_event_from(battery_guard)) {
            // drain every pending notification, otherwise the WaitSet wakes
            // again immediately and spins
            battery_listener.try_wait_all([](auto) { }).value();
            auto battery = battery_subscriber.receive().value();
            while (battery.has_value()) {
                std::cout << "battery: " << battery->payload().charge_percent << "%" << std::endl;
                battery = battery_subscriber.receive().value();
            }
        } else if (attachment_id.has_event_from(position_guard)) {
            // drain every pending notification, otherwise the WaitSet wakes
            // again immediately and spins
            position_listener.try_wait_all([](auto) { }).value();
            auto position = position_subscriber.receive().value();
            while (position.has_value()) {
                std::cout << "position: (" << position->payload().x << ", " << position->payload().y << ")"
                          << std::endl;
                position = position_subscriber.receive().value();
            }
        }

        return CallbackProgression::Continue;
    };

    waitset.wait_and_process(on_event).value();

    return 0;
}
// snippet:end
