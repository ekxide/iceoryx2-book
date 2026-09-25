fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start
    use core::time::Duration;
    use iceoryx2::prelude::*;
    use iceoryx2_integrations_zenoh_link_carrier::ZenohCarrier;
    use iceoryx2_link::Link;
    use iceoryx2_link_tunnel::Tunnel;

    const POLL_INTERVAL: Duration = Duration::from_millis(100);

    let config = iceoryx2::config::Config::global_config();
    let node = NodeBuilder::new().config(config).create::<ipc::Service>()?;
    let carrier = ZenohCarrier::create(zenoh::Config::default())?;
    let mut link = Link::new(node, Tunnel::new(carrier, config));

    while link.node().wait(POLL_INTERVAL).is_ok() {
        link.discover()?;
        link.propagate()?;
    }
    // snippet:end

    Ok(())
}
