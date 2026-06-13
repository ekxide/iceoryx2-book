"""Distance subscriber."""

from distance import Distance

# snippet:start node-and-service
import iceoryx2 as iox2

node = iox2.NodeBuilder.new().create(iox2.ServiceType.Ipc)

service = (
    node.service_builder(iox2.ServiceName.new("distance_to_obstacle"))
    .publish_subscribe(Distance)
    .open_or_create()
)
# snippet:end node-and-service

# snippet:start subscriber
subscriber = service.subscriber_builder().create()
# snippet:end subscriber

# snippet:start receive-loop
try:
    while True:
        node.wait(iox2.Duration.from_millis(100))
        while True:
            sample = subscriber.receive()
            if sample is not None:
                data = sample.payload()
                print("received distance:", data.contents)
            else:
                break

except iox2.NodeWaitFailure:
    print("exit")
# snippet:end receive-loop
