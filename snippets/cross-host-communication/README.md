# Cross-host communication snippets

* `larry_telemetry` — publishes `larry/battery` and `larry/position` and
   notifies on each.
* `dashboard` — wakes on those notifications via a `WaitSet` and prints the data.
* Rust only: `embedded_polled`, `embedded_reactive_sender`,
  `embedded_reactive_receiver` — drive the tunnel in-process
  (need a peer tunnel; cross-host).

Run `dashboard` and `larry_telemetry` on the same host (two terminals, any
order) to verify they communicate over shared memory. Expected, once per second:

```text
battery: 87%        # "87.0%" in C / Python
position: (1, 2)    # "(1.0, 2.0)" in C / Python
```

All commands run from the repository root.

## Rust

```sh
cd snippets
cargo run -p cross-host-communication --bin dashboard
cargo run -p cross-host-communication --bin larry_telemetry
```

## C++

```sh
just build-c-cxx-bindings
just build-cxx-snippets
cd snippets/cross-host-communication/cxx/build
./dashboard
./larry_telemetry
```

## C

```sh
just build-c-cxx-bindings
just build-c-snippets
cd snippets/cross-host-communication/c/build
./dashboard
./larry_telemetry
```

## Python

```sh
just build-python-bindings
source "$(poetry --project ../iceoryx2/iceoryx2-ffi/python env info --path)/bin/activate"
cd snippets/cross-host-communication/python
python -u dashboard.py
python -u larry_telemetry.py
```
