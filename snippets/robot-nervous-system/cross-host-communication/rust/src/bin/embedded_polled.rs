fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start
    use core::time::Duration;
    use iceoryx2::prelude::*;
    use iceoryx2_gateway::Gateway;
    use iceoryx2_integrations_zenoh_gateway_backend::ZenohBackend;

    const POLL_INTERVAL: Duration = Duration::from_millis(100);

    let mut gateway = Gateway::<ipc::Service, ZenohBackend<ipc::Service>>::new()
        .polled()
        .create()?;

    while gateway.node().wait(POLL_INTERVAL).is_ok() {
        gateway.discover()?;
        gateway.propagate()?;
    }
    // snippet:end

    Ok(())
}
