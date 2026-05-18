# Customize the platform abstraction

`iceoryx2`'s [upper layers](../fundamentals/layered-architecture.md) depend
on a platform abstraction layer (PAL) that lives in [`iceoryx2-pal`](
https://github.com/eclipse-iceoryx/iceoryx2/tree/main/iceoryx2-pal).
The PAL is currently POSIX-flavored, and has two parts: a set of compile-time
**settings** in [`iceoryx2-pal/configuration`](
https://github.com/eclipse-iceoryx/iceoryx2/tree/main/iceoryx2-pal/configuration),
and an implementation of the POSIX surface that `iceoryx2` uses in
[`iceoryx2-pal/posix`](
https://github.com/eclipse-iceoryx/iceoryx2/tree/main/iceoryx2-pal/posix).

There are a number of platforms supported out-of-the-box: **Linux**,
**FreeBSD**, **macOS**, **QNX**, and **Windows**.
Support for these implementations is maintained upstream — no porting effort
required. The PAL keeps pace with upstream changes as part of the normal
release cycle.

If support for another platform is required, two complementary override
mechanisms enable an integrator to substitute their own settings, a custom
POSIX implementation, or both, at build time. This approach however requires
taking ownership of both the initial port and the ongoing maintenance of
a custom platform.

Alternatively, get in touch with the [`iceoryx2` team](https://ekxide.io/contact)
if you would like to add support for additional platforms upstream.

## Compile-time Settings

[`iceoryx2-pal/configuration`](
https://github.com/eclipse-iceoryx/iceoryx2/tree/main/iceoryx2-pal/configuration)
exposes a small set of compile-time constants that `iceoryx2` uses.
The defaults are tailored to common operating systems, but a target may need
different values — for example, a platform whose temporary directory is
read-only, or one without a conventional `/tmp` at all.

The override is a single `.rs` file containing a `settings` module with the
expected constants:

```rust
pub mod settings {
    pub const GLOBAL_CONFIG_PATH: &[u8] = b"/etc";
    pub const USER_CONFIG_PATH: &[u8] = b".config";
    pub const TEMP_DIRECTORY: &[u8] = b"/my_tmp/";
    pub const TEST_DIRECTORY: &[u8] = b"/my_tmp/tests/";
    pub const SHARED_MEMORY_DIRECTORY: &[u8] = b"/dev/my_shm/";
    pub const PATH_SEPARATOR: u8 = b'/';
    pub const ROOT: &[u8] = b"/";
    pub const REQUIRED_SOCKET_DIRECTORY: Option<&[u8]> = None;
    pub const ICEORYX2_ROOT_PATH: &[u8] = b"/my_tmp/";
    pub const FILENAME_LENGTH: usize = 255;
    pub const PATH_LENGTH: usize = 255;
    pub const AT_LEAST_TIMING_VARIANCE: f32 = 0.25;
}
```

```{note}
The constants required by the `settings` module may evolve as `iceoryx2`
develops. Refer to [`iceoryx2-pal/configuration/src/lib.rs`](
https://github.com/eclipse-iceoryx/iceoryx2/blob/main/iceoryx2-pal/configuration/src/lib.rs)
for the current list.
```

The file lives anywhere on disk. To wire it in, set
`IOX2_CUSTOM_PLATFORM_CONFIGURATION_PATH` to its absolute path. For local
experimentation, exporting the env var directly works:

```console
export IOX2_CUSTOM_PLATFORM_CONFIGURATION_PATH=/abs/path/to/settings.rs
```

For anything beyond a one-off build, set it persistently via the project's
`.cargo/config.toml`:

```toml
[env]
IOX2_CUSTOM_PLATFORM_CONFIGURATION_PATH = "/abs/path/to/settings.rs"
```

`iceoryx2`'s build picks up the env var and substitutes the file in at compile time.
A successful build emits a warning naming the file in use:

```console
warning: iceoryx2-pal-configuration@x.y.z: Building with custom configuration: /abs/path/to/settings.rs
```

```{important}
Two gotchas to keep in mind:

1. **Clear `target/` after switching the override on or off.** A prior
   build with the default configuration is cached against a different
   `cfg`, and cargo will silently reuse it — the custom file appears to
   be ignored.
2. **The override is not available under Bazel builds.** It relies on
   `cargo`'s env-var-driven build script.
```

## POSIX Abstraction

[`iceoryx2-pal/posix`](
https://github.com/eclipse-iceoryx/iceoryx2/tree/main/iceoryx2-pal/posix)
is the implementation of the POSIX surface that `iceoryx2` uses. Built-in
implementations cover the supported platforms.
Targets that lack POSIX, or that need significant adjustments to a POSIX
implementation (different signal semantics, different shared-memory
primitives, an underlying RTOS instead of an OS), need their own
implementation in place of those provided upstream.

Implementing a custom POSIX abstraction can be a significant undertaking —
the POSIX surface spans file descriptors, signals, mmap, semaphores,
threading primitives, and more. The pragmatic starting point is to copy one of
the existing implementations in [`iceoryx2-pal/posix/src`](
https://github.com/eclipse-iceoryx/iceoryx2/tree/main/iceoryx2-pal/posix/src)
from upstream and adapt each primitive to the platform's actual capabilities.

Wiring is the same shape as the settings override, with a different env
var pointing at the directory containing your implementation:

```console
export IOX2_CUSTOM_POSIX_PLATFORM_PATH=/abs/path/to/posix
```

Or persistently in `.cargo/config.toml`:

```toml
[env]
IOX2_CUSTOM_POSIX_PLATFORM_PATH = "/abs/path/to/posix"
```

A successful build emits a warning naming the directory in use:

```console
warning: iceoryx2-pal-posix@x.y.z: Building with custom POSIX abstraction at: /abs/path/to/posix/
```

```{important}
The same two gotchas as the settings override apply: clear `target/` when
switching the override on or off, and the override is unavailable under
Bazel builds.
```

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Platform Settings Source
:link: https://github.com/eclipse-iceoryx/iceoryx2/blob/main/iceoryx2-pal/configuration/src/lib.rs
:link-type: url
:shadow: none

View the current settings included in the platform configuration.
```

```{grid-item-card} POSIX PAL Source
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/iceoryx2-pal/posix/src
:link-type: url
:shadow: none

View the currently-available POSIX PAL implementations
```
````
