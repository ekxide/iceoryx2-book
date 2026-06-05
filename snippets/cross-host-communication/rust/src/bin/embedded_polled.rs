fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start
    use core::time::Duration;
    use iceoryx2::prelude::*;
    use iceoryx2_integrations_zenoh_tunnel_backend::ZenohBackend;
    use iceoryx2_services_tunnel::Tunnel;

    const POLL_INTERVAL: Duration = Duration::from_millis(100);

    let mut tunnel = Tunnel::<ipc::Service, ZenohBackend<ipc::Service>>::new()
        .polled()
        .create()?;

    while tunnel.node().wait(POLL_INTERVAL).is_ok() {
        tunnel.discover()?;
        tunnel.propagate()?;
    }
    // snippet:end

    Ok(())
}
