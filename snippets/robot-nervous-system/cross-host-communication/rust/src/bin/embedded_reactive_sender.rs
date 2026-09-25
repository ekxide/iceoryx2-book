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

    // wake whenever Larry publishes locally, so freshly produced samples are
    // pushed out promptly
    let battery_listener = link
        .node()
        .service_builder(&"larry/battery".try_into()?)
        .event()
        .open_or_create()?
        .listener_builder()
        .create()?;
    let position_listener = link
        .node()
        .service_builder(&"larry/position".try_into()?)
        .event()
        .open_or_create()?
        .listener_builder()
        .create()?;

    let waitset = WaitSetBuilder::new().create::<ipc::Service>()?;
    let _battery_guard = waitset.attach_notification(&battery_listener)?;
    let _position_guard = waitset.attach_notification(&position_listener)?;

    waitset.wait_and_process(|_| {
        let _ = battery_listener.try_wait(|_| {});
        let _ = position_listener.try_wait(|_| {});
        let _ = link.discover();
        let _ = link.propagate();

        CallbackProgression::Continue
    })?;
    // snippet:end

    Ok(())
}
