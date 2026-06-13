fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start
    use iceoryx2::prelude::*;
    use iceoryx2_integrations_zenoh_tunnel_backend::ZenohBackend;
    use iceoryx2_services_tunnel::Tunnel;

    // reactive mode hands back a listener that wakes when the backend has
    // delivered something to ingest from the wire
    let (mut tunnel, listener) = Tunnel::<ipc::Service, ZenohBackend<ipc::Service>>::new()
        .reactive()
        .create()?;

    while listener.blocking_wait(|_| {}).is_ok() {
        tunnel.discover()?;
        tunnel.propagate()?;
    }
    // snippet:end

    Ok(())
}
