mod twist;

use iceoryx2::prelude::*;
use twist::Twist;

const MAX_VELOCITY_M_PER_S: f64 = 1.0;

fn limit(twist: &Twist) -> Twist {
    let mut limited = twist.clone();
    limited.0.linear.x = limited
        .0
        .linear
        .x
        .clamp(-MAX_VELOCITY_M_PER_S, MAX_VELOCITY_M_PER_S);
    limited
}

fn main() -> Result<(), Box<dyn core::error::Error>> {
    set_log_level_from_env_or(LogLevel::Info);

    let node = NodeBuilder::new().create::<ipc::Service>()?;

    let subscriber = node
        .service_builder(&"CmdVel".try_into()?)
        .publish_subscribe::<Twist>()
        .open_or_create()?
        .subscriber_builder()
        .create()?;
    let listener = node
        .service_builder(&"CmdVel".try_into()?)
        .event()
        .open_or_create()?
        .listener_builder()
        .create()?;

    let publisher = node
        .service_builder(&"CmdVelLimited".try_into()?)
        .publish_subscribe::<Twist>()
        .open_or_create()?
        .publisher_builder()
        .create()?;
    let notifier = node
        .service_builder(&"CmdVelLimited".try_into()?)
        .event()
        .open_or_create()?
        .notifier_builder()
        .create()?;

    let waitset = WaitSetBuilder::new().create::<ipc::Service>()?;
    let _guard = waitset.attach_notification(&listener)?;

    let on_event = |_: WaitSetAttachmentId<ipc::Service>| {
        // drain every pending notification, otherwise the WaitSet wakes
        // again immediately and spins
        listener.try_wait(|_| {}).unwrap();

        while let Ok(Some(sample)) = subscriber.receive() {
            let received = sample.payload();
            let limited = limit(received);

            coutln!(
                "limited linear.x from {} to {}",
                received.0.linear.x,
                limited.0.linear.x
            );

            publisher
                .loan_uninit()
                .unwrap()
                .write_payload(limited)
                .send()
                .unwrap();
            notifier.notify().unwrap();
        }

        CallbackProgression::Continue
    };

    waitset.wait_and_process(on_event)?;

    Ok(())
}
