fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start
    use iceoryx2::prelude::*;
    use iceoryx2_integrations_zenoh_link_carrier::ZenohCarrier;
    use iceoryx2_link::Link;
    use iceoryx2_link_tunnel::Tunnel;

    let config = iceoryx2::config::Config::global_config();
    let node = NodeBuilder::new().config(config).create::<ipc::Service>()?;
    let carrier = ZenohCarrier::create(zenoh::Config::default())?;
    let mut link = Link::new(node, Tunnel::new(carrier, config));

    // the listener wakes when the carrier has delivered something to ingest
    // from the wire
    let listener = link.listener()?;

    while listener.blocking_wait(|_| {}).is_ok() {
        link.discover()?;
        link.propagate()?;
    }
    // snippet:end

    Ok(())
}
