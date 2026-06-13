fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start
    use iceoryx2::prelude::*;
    use iceoryx2_integrations_zenoh_tunnel_backend::ZenohBackend;
    use iceoryx2_services_tunnel::Tunnel;

    let mut tunnel = Tunnel::<ipc::Service, ZenohBackend<ipc::Service>>::new()
        .polled()
        .create()?;

    // wake whenever Larry publishes locally, so freshly produced samples are
    // pushed out promptly
    let battery_listener = tunnel
        .node()
        .service_builder(&"larry/battery".try_into()?)
        .event()
        .open_or_create()?
        .listener_builder()
        .create()?;
    let position_listener = tunnel
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
        let _ = tunnel.discover();
        let _ = tunnel.propagate();

        CallbackProgression::Continue
    })?;
    // snippet:end

    Ok(())
}
