# Reproducible build/verification helpers for the iceoryx2 book snippets.

iox2_src := env_var_or_default("IOX2_SRC", justfile_directory() / ".." / "iceoryx2")
install_prefix := iox2_src / "target" / "ff" / "cc" / "install"

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
