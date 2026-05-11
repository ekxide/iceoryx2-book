fn main() -> Result<(), Box<dyn core::error::Error>> {
    // snippet:start
    use core::time::Duration;
    use iceoryx2::prelude::*;

    const TARGET_PRICE: f64 = 100.0;

    #[derive(Debug, Clone, Copy, ZeroCopySend)]
    #[repr(C)]
    struct PriceTick {
        instrument_id: u32,
        price: f64,
    }

    let node = NodeBuilder::new()
        .name(&"MarketDataConsumer".try_into()?)
        .create::<ipc::Service>()?;

    let ticks_service = node
        .service_builder(&"market/ticks".try_into()?)
        .publish_subscribe::<PriceTick>()
        .open_or_create()?;

    let orders_service = node
        .service_builder(&"market/orders".try_into()?)
        .publish_subscribe::<u32>()
        .open_or_create()?;

    let subscriber = ticks_service.subscriber_builder().create()?;
    let orders_publisher = orders_service.publisher_builder().create()?;

    while node.wait(Duration::ZERO).is_ok() {
        while let Some(tick) = subscriber.receive()? {
            if tick.price > TARGET_PRICE {
                orders_publisher.send_copy(tick.instrument_id)?;
            }
        }
    }
    // snippet:end

    Ok(())
}
