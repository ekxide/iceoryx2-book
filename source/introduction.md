# Introduction

`iceoryx2` is a shared-memory communication middleware that enables true
zero-copy communication between applications on the same host.

By abstracting the complexity of shared memory behind simple APIs, it enables
developers to focus on their domain-specific problems while seamlessly
achieving low-latency, efficient communication.

```{figure} /images/grand-vision.svg
:alt: vision for iceoryx2
:align: center
:name: fig-grand-vision

Distributed, Portable and Safety-Certifiable Systems with `iceoryx2`
```

The vision for `iceoryx2` is to provide a universal communication foundation
that spans diverse technology ecosystems and meets safety-critical
requirements.

Support for multiple languages and platforms is currently available. Refer to
the [API reference index](https://eclipse-iceoryx.github.io/iceoryx2/)
for the list of supported languages.

Upcoming on the development roadmap is support for hypervisors, real-time
platforms, cloud applications, safety-critical domains, as well as
additional language bindings.

## Further Reading

````{grid} 1 1 2 3
:gutter: 2

```{grid-item-card} Understand Shared Memory
:link: /fundamentals/shared-memory
:link-type: doc
:shadow: none

Build an intuition for what shared memory communication entails.
```

```{grid-item-card} Browse the API Reference
:link: https://eclipse-iceoryx.github.io/iceoryx2/
:link-type: url
:shadow: none

Browse the API references of supported languages.
```

```{grid-item-card} Explore the Code
:link: https://github.com/eclipse-iceoryx/iceoryx2
:link-type: url
:shadow: none

See how the architecture is realized in the code in the repository.
```

````
