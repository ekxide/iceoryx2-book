Getting Started
===============

Meet Larry, our imaginary robot. Larry’s got an ultrasonic sensor, a camera, and
enough algorithms to find his way from A to B without hitting a wall. The catch?
Those algorithms and sensors are already built, but they live in different
processes, and maybe even on different machines. Your job is to wire it all
together so Larry actually does something useful.

That’s where iceoryx2 comes in. In this tutorial, we’ll build the communication
layer that connects Larry’s brain, eyes, and wheels. Along the way, we’ll look
at publish–subscribe messaging, events, and request–response patterns.

And because a real system is more than just “moving data around,” we’ll also set
things up so you can debug and introspect Larry, record sensor data, and replay
it later while you cook up new algorithms.

Think of it as putting Larry’s nervous system online—without the medical degree.

.. toctree::
   :maxdepth: 1

   publish-subscribe
   event
   event-driven-communication
   request-response
   blackboard
   iox2-debugging
