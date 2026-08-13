Gateway to ROS 2
================

.. admonition:: Learning Objectives

   This series of articles shows how to compose a system from both ROS 2 and
   ``iceoryx2`` applications, and how to choose between the approaches for
   connecting them.

Consider the following: you and your team have been working on a robotics
project. An idea so novel, unique and brilliant, that it will disrupt the
industry and change the way we all live. Forever! You have spent the last few
years building a prototype in ROS 2, and it is finally complete. Functionally,
it works.

But it is not yet perfect. Responsiveness could be snappier. Communication
latency has become a bottleneck, especially for larger payloads. Or safety
certification has reared its head as a barrier to production.

With ``iceoryx2`` it is possible to address these issues, but the ROS 2
stack need not be completely discarded in all cases. The ROS 2 gateway
makes it possible to compose a hybrid system, consisting of both ROS 2 and
iceoryx2 applications.

.. mermaid::
   :caption: Gateway bridging ROS 2 and iceoryx2 domains
   :alt: ROS 2 nodes communicate over DDS, iceoryx2 applications over shared memory, and the Gateway forwards traffic between the two

   %%{init: {"flowchart": {"subGraphTitleMargin": {"top": 10, "bottom": 8}}} }%%
   flowchart LR
       subgraph ros2["ROS 2"]
           n1["Node"]:::external
           n2["Node"]:::external
       end

       gw["Gateway"]:::gateway

       subgraph iox2["iceoryx2"]
           a1["Application"]
           a2["Application"]
       end

       n1 <-- "DDS" --> n2
       n2 <-- "DDS" --> gw
       gw <-- "SHM" --> a1
       a1 <-- "SHM" --> a2

In this tutorial series we will cover how to use the ROS 2 gateway. First we
will go over basic usage, then take a look at some different approaches for
exchanging payloads across the boundary. The approach that fits your system
best is a case-by-case design decision:

.. toctree::
   :maxdepth: 1

   gateway-basics
   plain-struct-as-payload
   flatbuffer-as-payload
   cdr-bytes-as-payload
