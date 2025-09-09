# Quickstart

## Run an Example

1. Clone the [`iceoryx2`](https://github.com/eclipse-iceoryx/iceoryx2/)
repository:
    ```console
    git clone git@github.com:eclipse-iceoryx/iceoryx2.git
    ```
1. Navigate to the cloned repository:
    ```console
    cd iceoryx2
    ```
1. Try an example from the [examples folder](https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/rust)
    1. Terminal A
       ```console
       cargo run --example publish_subscribe_subscriber
       ```
    1. Terminal B
        ```console
       cargo run --example publish_subscribe_publisher
        ```

## Add `iceoryx2` to your project

````{tab-set}

```{tab-item} Rust

If working with `cargo`, then `iceoryx2` can be added to your project with:

    cargo add iceoryx2

Alternatively, you can manually add the desired version to your project's
Cargo.toml:

    [dependencies]
    iceoryx2 = "0.7.0"

```

```{tab-item} Python

The `iceoryx2` Python bindings can be pulled into your project from the
Python Package Index. You can install it into your environment with:

    pip install iceoryx2

Alternatively, the package can be added as a dependency using whichever
dependency management solution used by your project.

**For Poetry projects:**

The dependency can be added with:

    poetry add iceoryx2

**For projects using `requirements.txt`:**

Add the desired version to your `requirements.txt`:

    iceoryx2==0.7.0

```

```{tab-item} C++

In order to use `iceoryx2` in your C++ project, [the C++ bindings](
https://github.com/eclipse-iceoryx/iceoryx2/tree/main/iceoryx2-cxx) to
`iceoryx2` must be built.

From the `iceoryx2` repository root, execute the following:

    cmake -S . -B target/ff/cc/build
    cmake --build target/ff/cc/build
    cmake --install target/ff/build --prefix target/ff/install # or some other location

Then, make the installed headers and libraries available to your project using one of these methods:

**For CMake projects:**

Set the `CMAKE_PREFIX_PATH` to include the install location:

    set(CMAKE_PREFIX_PATH "/path/to/prefix" ${CMAKE_PREFIX_PATH})
    find_package(iceoryx2 REQUIRED)
    target_link_libraries(your_target iceoryx2-cxx)

Or pass it via command line:
    
    cmake -DCMAKE_PREFIX_PATH=/path/to/prefix ...

**For other build systems:**

- Add the include directory: `/path/to/prefix/include`
- Add the library directory: `/path/to/prefix/lib`
- Link against the built iceoryx2 library

```

```{tab-item} C
In order to use `iceoryx2` in your C project, [the C bindings](
https://github.com/eclipse-iceoryx/iceoryx2/tree/main/iceoryx2-c) to
`iceoryx2` must be built.

From the `iceoryx2` repository root, execute the following:

    cmake -S . -B target/ff/cc/build -DBUILD_CXX=OFF
    cmake --build target/ff/cc/build
    cmake --install target/ff/build --prefix target/ff/install # or some other location

Then, make the installed headers and libraries available to your project using one of these methods:

**For CMake projects:**

Set the `CMAKE_PREFIX_PATH` to include the install location:

    set(CMAKE_PREFIX_PATH "/path/to/prefix" ${CMAKE_PREFIX_PATH})
    find_package(iceoryx2 REQUIRED)
    target_link_libraries(your_target iceoryx2-c)

Or pass it via command line:
    
    cmake -DCMAKE_PREFIX_PATH=/path/to/prefix ...

**For other build systems:**

- Add the include directory: `/path/to/prefix/include`
- Add the library directory: `/path/to/prefix/lib`
- Link against the built iceoryx2 library

```

````
