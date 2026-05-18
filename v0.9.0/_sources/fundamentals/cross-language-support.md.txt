# Cross-Language Support

```{figure} /images/cross-language-support.svg
:alt: cross-language communication over shared memory
:align: center
:name: fig-cross-language-support

Cross-Language Zero-Copy Communication
```

The core of `iceoryx2` is written in Rust, providing a robust foundation for
zero-copy inter-process communication. An FFI (Foreign Function Interface)
layer provides API bindings that enable integration with other programming
languages.

This architecture enables applications written in different languages to
communicate with each other through `iceoryx2`'s shared memory system.
Communication works seamlessly as long as the payload type can be identified
and its memory layout can be interpreted consistently by both the sending and
receiving applications.

## Relocatable Data Types

Ensuring consistent memory layout interpretation across different programming
languages can be challenging. Languages may have different padding rules,
alignment requirements, or data type size assumptions that can lead to data
corruption or misinterpretation when sharing memory directly.

For primitive data types, the complexity can be manageable, but can become
challenging when structuring more complex data. Different languages could
handle composite types, arrays, and nested structures in incompatible ways,
making direct memory sharing more difficult.

To address these challenges, a collection of generic cross-language compatible
data structures called "relocatable data types" are currently in development.
These structures will provide standardized, safe primitives and containers
specifically designed for cross-language communication via shared memory
to tackle the complexity of manual memory layout management.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Example: Cross-language Communication of Basic Types (Rust)
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/rust/cross_language_communication_basics
:link-type: url
:shadow: none

Send basic types over shared memory from Rust to applications in other languages.
```

```{grid-item-card} Example: Cross-language Communication of Basic Types (Python)
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/python/cross_language_communication_basics
:link-type: url
:shadow: none

Send basic types over shared memory from Python to applications in other languages.
```

```{grid-item-card} Example: Cross-language Communication of Basic Types (C++)
:link: https://github.com/eclipse-iceoryx/iceoryx2/tree/main/examples/cxx/cross_language_communication_basics
:link-type: url
:shadow: none

Send basic types over shared memory from C++ to applications in other languages.
```

````
