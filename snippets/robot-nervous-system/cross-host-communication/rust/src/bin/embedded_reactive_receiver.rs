fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start
    use iceoryx2::prelude::*;
    use iceoryx2_gateway::Gateway;
    use iceoryx2_integrations_zenoh_gateway_backend::ZenohBackend;

    // reactive mode hands back a listener that wakes when the backend has
    // delivered something to ingest from the wire
    let (mut gateway, listener) = Gateway::<ipc::Service, ZenohBackend<ipc::Service>>::new()
        .reactive()
        .create()?;

    while listener.blocking_wait(|_| {}).is_ok() {
        gateway.discover()?;
        gateway.propagate()?;
    }
    // snippet:end

    Ok(())
}
