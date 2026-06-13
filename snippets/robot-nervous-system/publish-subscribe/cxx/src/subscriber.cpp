#include "distance.hpp"
#include "iox2/iceoryx2.hpp"

#include <iostream>

auto main() -> int {
    // snippet:start node-and-service
    using namespace iox2;

    auto node = NodeBuilder().create<ServiceType::Ipc>().value();

    auto service = node.service_builder(ServiceName::create("distance_to_obstacle").value())
                       .publish_subscribe<Distance>()
                       .open_or_create()
                       .value();
    // snippet:end node-and-service

    // snippet:start subscriber
    auto subscriber = service.subscriber_builder().create().value();
    // snippet:end subscriber

    // snippet:start receive-loop
    while (node.wait(iox2::bb::Duration::from_millis(100)).has_value()) {
        auto sample = subscriber.receive().value();
        while (sample.has_value()) {
            std::cout << "received distance: " << sample->payload() << std::endl;
            sample = subscriber.receive().value();
        }
    }
    // snippet:end receive-loop

    return 0;
}
