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
[`iceoryx2-loggers`](
https://github.com/eclipse-iceoryx/iceoryx2/blob/main/iceoryx2-log/loggers/Cargo.toml)
crate. The default logger is automatically used by the logging API unless
overridden at runtime by your application.

The `iceoryx2` crate sets a [sane default](
https://github.com/eclipse-iceoryx/iceoryx2/blob/main/iceoryx2/Cargo.toml)
for the logger backend when built with default features enabled. If this
default suits your use case, no additional configuration is needed.

However, if you want to build without default features enabled (such as when
building for `no_std` targets), you need to manually enable the appropriate
features on the `iceoryx2-loggers` crate.

You can do this in your `Cargo.toml` by adding `iceoryx2-loggers` as a
dependency:

```toml
iceoryx2-loggers = { version = "0.7.0", default-features = false, features = ["std", "console"] }
```

## Overriding the default logger

You can also override the default logger at runtime. This is most useful when
you want to provide a custom logger implementation rather than use those
provided by `iceoryx2`. This approach is slightly less ergonomic, as you must
remember to set the logger in your application before any `iceoryx2` APIs are
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
