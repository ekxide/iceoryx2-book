# Configure the logging backend

`iceoryx2` provides the capability to switch out the logging backend used by
its logging API.

Specifically, this allows you to configure where log messages from macros such
as `info!` and `fail!` (used throughout the `iceoryx2` codebase) are written
to.

There are two ways to configure the logging backend:

1. Specifying a default logger at compile time
2. Overriding the default logger with a custom logger at runtime

## Specifying a default logger

The default logger is configured via feature flags set on the
[`iceoryx2-bb-loggers`](
https://github.com/eclipse-iceoryx/iceoryx2/blob/main/iceoryx2-bb/loggers/Cargo.toml)
crate, which are re-exported in the [`iceoryx2`](
https://github.com/eclipse-iceoryx/iceoryx2/blob/main/iceoryx2/Cargo.toml)
crate.

The default logger is automatically used by the logging API unless
overridden at runtime by your application.

The `iceoryx2` crate sets a [sane default](
https://github.com/eclipse-iceoryx/iceoryx2/blob/main/iceoryx2/Cargo.toml)
for the logger backend when built with default features enabled. If this
default suits your use case, no additional configuration is needed.

However, if you want to build without default features enabled (such as when
building for `no_std` targets), corresponding features need to be enabled.

This can be done in your `Cargo.toml` using the `std` features and re-exported
logger features:

### `std` builds

```toml
iceoryx2 = { version = "X.Y.Z", default-features = false, features = ["std", "console"] }
```

### `no_std` builds

```toml
iceoryx2 = { version = "X.Y.Z", default-features = false, features = ["console"] }
```

## Overriding the default logger

The default logger can also be overwritten at runtime. This is most useful when
wanting to provide a custom logger implementation rather than use those
provided by `iceoryx2`. This approach is slightly less ergonomic, as the logger
must be set at runtime in your application before any `iceoryx2` APIs are
called.

```rust
use iceoryx2::prelude::*;

static LOGGER: MyLogger = MyLogger::new();

fn main() {
    set_logger(&LOGGER);
    // ...
}
```

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} View available Loggers
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/iceoryx2-log/loggers
:link-type: url
:shadow: none

See how the available loggers are implemented and get an idea how to implement
your own.
```

```{grid-item-card} Build for no_std targets
:link: /how-to/build-for-nostd-targets
:link-type: doc
:shadow: none

See how configuring the default logger relates to building for `no_std` targets.
```

```{grid-item-card} View logger configuration for no_std on POSIX
:link: https://github.com/eclipse-iceoryx/iceoryx2/blob/main/examples/nostd/posix/rust/Cargo.toml
:link-type: url
:shadow: none

See the default logger selected for the `no_std` POSIX examples.
```


````
