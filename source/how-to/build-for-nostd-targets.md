# Build for `no_std` targets

```{important}
Support for `no_std` targets is still in an immature state.

* `no_std` builds with POSIX support should be fully functional; however, they
  have not yet been thoroughly validated in production use
* `no_std` builds for bare metal are currently proof-of-concept and do not
  yet support all features
```

## Configuring the `iceoryx2` build for `no_std`

As is convention in the Rust ecosystem, building for `no_std` is done by
disabling the `std` feature. You can do this when specifying the dependency
in your `Cargo.toml`:

```toml
iceoryx2 = { version = "0.8.0", default-features = false }
```

## Selecting a default logger

Disabling the default features on the `iceoryx2` crate also disables the
features that select the default logger in the `iceoryx2-loggers` crate.
As a result, all log messages are discarded.

For POSIX targets such as QNX 8, which currently does not have `std` support
upstream, a console logger implementation that uses the POSIX API is
available and can be selected by configuring the dependency in your
`Cargo.toml`:

```toml
iceoryx2-loggers = { version = "0.8.0", default-features = false, features = ["posix", "console"]}
```

For bare metal targets, there are no alternatives available yet, but stay tuned!

## Setting up your `no_std` application

When building a `no_std` application, you must provide implementations for
certain core functionality that the standard library would normally handle.

Take a look at the [`no_std` examples](
https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/nostd) for
a reference on how this could be done for both POSIX and bare metal targets.

### Global allocator

Since `iceoryx2` uses the `alloc` crate, you must specify a global
allocator somewhere in the application:

```rust
#[global_allocator]
static GLOBAL: MyGlobalAllocator = MyGlobalAllocator::new();
```

### Panic handler

All `no_std` applications must define a panic handler using the
`#[panic_handler]` attribute:

```rust
#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
    // Your custom panic logic.
    loop {
        core::hint::spin_loop();
    }
}
```

### Application entry point

In a `no_std` environment, you must define an appropriate entry point
for your platform. This can be done using the `#![no_main]` and
`#[no_mangle]` attributes.

The `#![no_main]` attribute [prevents the compiler from emitting the `main`
symbol](
https://doc.rust-lang.org/reference/crates-and-source-files.html#the-no_main-attribute),
allowing you to define your own entry point that will be called by
the platform's startup code. The `#[no_mangle]` attribute preserves the
function name in the compiled binary so it can be located by the linker.

For POSIX targets, define a C-compatible `main` function that returns an
exit code:

```rust
#![no_std]
#![no_main]

#[no_mangle]
extern "C" fn main() -> i32 {
    // Your application code here
    
    0 // Success
}
```

The startup code in the C runtime that POSIX platforms provide will
automatically jump to this entry point after completing its initialization.

For bare metal targets, define a function that integrates with your platform's
startup code, for example:

```rust
#![no_std]
#![no_main]

#[no_mangle]
pub extern "C" fn entrypoint() -> ! {
    // Your application code here
    
    loop {}  // Or handle exit in your platform code
}
```

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} The Rust Embedded Book
:link: https://docs.rust-embedded.org/book/intro/no-std.html
:link-type: url
:shadow: none

Documentation on the `no_std` Rust environment from the Rust Embedded Book.
```

```{grid-item-card} Reference Examples for POSIX targets (Rust)
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/nostd/posix/rust
:link-type: url
:shadow: none

Reference `no_std` examples for targets with POSIX support.
```

```{grid-item-card} Reference Examples for bare metal targets (Rust)
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/nostd/bare-metal/rust
:link-type: url
:shadow: none

Reference `no_std` examples for bare metal targets.
```

````
