# Reproducible build/verification helpers for the iceoryx2 book snippets.

iox2_src := env_var_or_default("IOX2_SRC", justfile_directory() / ".." / "iceoryx2")
install_prefix := iox2_src / "target" / "ff" / "cc" / "install"
ros2_ws := justfile_directory() / "snippets" / "gateway-to-ros-2" / "ws"

default:
    @just --list

# ---------------------------------------------------------------------------
# Build the dependencies the compile checks link against
# ---------------------------------------------------------------------------

# Build & install the iceoryx2 C and C++ bindings (one CMake build produces both).
build-c-cxx-bindings:
    cmake -S {{iox2_src}} -B {{iox2_src}}/target/ff/cc/build \
        -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX={{install_prefix}}
    cmake --build {{iox2_src}}/target/ff/cc/build --config Release -j
    cmake --install {{iox2_src}}/target/ff/cc/build --config Release

# Build the iceoryx2 Python bindings into the bindings project's venv.
build-python-bindings:
    poetry --project {{iox2_src}}/iceoryx2-ffi/python install
    poetry --project {{iox2_src}}/iceoryx2-ffi/python build-into-venv

# ---------------------------------------------------------------------------
# Compile checks — verify the snippets build
# ---------------------------------------------------------------------------

# Rust: compiles (cargo check).
build-rust-snippets:
    cd snippets && cargo check --workspace --all-targets

# C++: every example's cxx/ project compiles against the installed bindings.
build-cxx-snippets:
    #!/usr/bin/env sh
    set -e
    for dir in $(find snippets -type d -name cxx -not -path '*/build/*'); do
        [ -f "$dir/CMakeLists.txt" ] || continue
        echo ">> $dir"
        cmake -S "$dir" -B "$dir/build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH={{install_prefix}}
        cmake --build "$dir/build" --config Release
    done

# C: every example's c/ project compiles against the installed bindings.
build-c-snippets:
    #!/usr/bin/env sh
    set -e
    for dir in $(find snippets -type d -name c -not -path '*/build/*'); do
        [ -f "$dir/CMakeLists.txt" ] || continue
        echo ">> $dir"
        cmake -S "$dir" -B "$dir/build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH={{install_prefix}}
        cmake --build "$dir/build" --config Release
    done

# ROS 2: the gateway-to-ros-2 colcon packages compile (requires a sourced ROS 2
# environment plus the colcon-cargo, colcon-ros-cargo, vcstool and
# cargo-ament-build tools).
build-ros2-snippets:
    #!/usr/bin/env bash
    set -eo pipefail
    if [ -z "${ROS_DISTRO:-}" ]; then
        echo "Error: no sourced ROS 2 environment" >&2
        echo "  Source your ROS 2 setup first, e.g.: source /opt/ros/jazzy/setup.bash" >&2
        exit 1
    fi
    cd "{{ros2_ws}}"
    vcs import --skip-existing src < "${ROS_DISTRO}.repos"
    colcon build --packages-up-to std_msgs geometry_msgs rosidl_generator_rs
    source install/setup.bash
    colcon build --packages-select twist_limiter chatter_relay gateway_basics

# Python: every example's python/ scripts are syntactically valid (py_compile).
build-python-snippets:
    #!/usr/bin/env sh
    set -e
    for dir in $(find snippets -type d -name python -not -path '*/build/*'); do
        ls "$dir"/*.py >/dev/null 2>&1 || continue
        echo ">> $dir"
        ( cd "$dir" && poetry --project {{iox2_src}}/iceoryx2-ffi/python run python -m py_compile *.py )
    done

# ---------------------------------------------------------------------------
# Lint
# ---------------------------------------------------------------------------

# Rust: clippy-clean (warnings treated as errors).
clippy-rust:
    cd snippets && cargo clippy --workspace --all-targets -- -D warnings

# ---------------------------------------------------------------------------
# Formatting — apply (no suffix) or verify (-check), per toolchain
# ---------------------------------------------------------------------------

# Rust: reformat in place with rustfmt.
format-rust:
    cd snippets && cargo fmt --all

# Rust: verify rustfmt formatting without modifying files (used by CI).
format-rust-check:
    cd snippets && cargo fmt --all --check

# ROS 2: verify rustfmt formatting of the colcon packages, which are not
# members of the snippets cargo workspace (used by CI).
format-ros2-snippets-check:
    find {{ros2_ws}}/src/twist_limiter {{ros2_ws}}/src/chatter_relay \
        {{ros2_ws}}/src/gateway_basics \
        \( -name build -o -name target \) -prune -o -type f -name '*.rs' \
        -print0 | xargs -0 rustfmt --edition 2024 --check

# C/C++: reformat in place with clang-format (all examples, per the root .clang-format).
format-c-cxx:
    find snippets -name build -prune -o -type f \
        \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.hpp' \) \
        -print0 | xargs -0 clang-format -i

# C/C++: verify clang-format formatting without modifying files (used by CI).
format-c-cxx-check:
    find snippets -name build -prune -o -type f \
        \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.hpp' \) \
        -print0 | xargs -0 clang-format --dry-run --Werror

# ---------------------------------------------------------------------------

# C/C++/Python compile checks need the bindings built first (build-*-bindings).
# Run every check: formatting, lint, and compile for each language.
check-all: format-rust-check format-c-cxx-check clippy-rust build-rust-snippets build-cxx-snippets build-c-snippets build-python-snippets
