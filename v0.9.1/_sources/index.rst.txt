The iceoryx2 Book
=================

.. meta::
   :description: The iceoryx2 Book — fundamentals, tutorials, and how-tos for building safe, reliable and performant zero-copy systems with iceoryx2.
   :property=og:description: The iceoryx2 Book — fundamentals, tutorials, and how-tos for building safe, reliable and performant zero-copy systems with iceoryx2.

.. raw:: html

   <div class="ix-hero">
     <div class="ix-hero__mark" aria-hidden="true">
       <svg viewBox="48 41.8 106.5 76.2" width="118" height="84" focusable="false">
         <g stroke="currentColor" stroke-width="1.2" stroke-linejoin="round" vector-effect="non-scaling-stroke">
           <path fill="#588bb5" transform="matrix(0.1,0,0,-0.1,0,200)" d="M 848.671875 1037.96875 L 769.335938 1174.023438 L 749.492188 974.179688 Z M 848.671875 1037.96875 "/>
           <path fill="#588bb5" transform="matrix(0.1,0,0,-0.1,0,200)" d="M 756.5625 1189.609375 L 718.515625 1258.398438 L 681.679688 1218.710938 Z M 756.5625 1189.609375 "/>
           <path fill="#b3e6ef" transform="matrix(0.1,0,0,-0.1,0,200)" d="M 1444.53125 1230.859375 L 1303.828125 1216.25 L 1105.859375 1070.9375 Z M 1444.53125 1230.859375 "/>
           <path fill="#b3e6ef" transform="matrix(0.1,0,0,-0.1,0,200)" d="M 730.078125 998.125 L 737.1875 1067.539062 L 521.015625 862.539062 Z M 730.078125 998.125 "/>
           <path fill="#588bb5" transform="matrix(0.1,0,0,-0.1,0,200)" d="M 1445.703125 1211.640625 L 1384.570312 1180.15625 L 1362.65625 1044.101562 Z M 1445.703125 1211.640625 "/>
           <path fill="#b3e6ef" d="M 86.429688 94.785156 C 86.75 94.785156 101.570312 67.914062 101.570312 67.914062 L 73.390625 73.148438 L 86.429688 94.785156 "/>
           <path fill="none" transform="matrix(0.1,0,0,-0.1,0,200)" d="M 864.296875 1052.148438 C 867.5 1052.148438 1015.703125 1320.859375 1015.703125 1320.859375 L 733.90625 1268.515625 Z M 864.296875 1052.148438 "/>
           <path fill="#b3e6ef" transform="matrix(0.1,0,0,-0.1,0,200)" d="M 1267.734375 1252.304688 L 1315.898438 1531.015625 L 1035 1325.9375 Z M 1267.734375 1252.304688 "/>
           <path fill="#588bb5" d="M 92.820312 87.738281 L 106.863281 92.855469 L 128.308594 77.566406 L 102.914062 69.160156 L 92.820312 87.738281 "/>
           <path fill="none" transform="matrix(0.1,0,0,-0.1,0,200)" d="M 928.203125 1122.617188 L 1068.632812 1071.445312 L 1283.085938 1224.335938 L 1029.140625 1308.398438 Z M 928.203125 1122.617188 "/>
           <path fill="#588bb5" d="M 131.261719 57.824219 L 150.371094 56.046875 L 150.371094 53.316406 L 132.828125 46.078125 L 131.261719 57.824219 "/>
           <path fill="none" transform="matrix(0.1,0,0,-0.1,0,200)" d="M 1312.617188 1421.757812 L 1503.710938 1439.53125 L 1503.710938 1466.835938 L 1328.28125 1539.21875 Z M 1312.617188 1421.757812 "/>
           <path fill="#b3e6ef" transform="matrix(0.1,0,0,-0.1,0,200)" d="M 1314.53125 1406.40625 L 1365.234375 1410.273438 L 1285.273438 1244.53125 Z M 1314.53125 1406.40625 "/>
           <path fill="#588bb5" transform="matrix(0.1,0,0,-0.1,0,200)" d="M 1297.96875 1539.023438 L 851.40625 1539.0625 L 1243.398438 1500.039062 Z M 1297.96875 1539.023438 "/>
         </g>
       </svg>
     </div>
     <p class="ix-hero__eyebrow"><span class="ix-hero__dot"></span>The <span class="ix-hero__word">iceoryx<span class="ix-hero__two">2</span></span> Book</p>
     <h1 class="ix-hero__title">The guide to iceoryx<span class="ix-hero__two">2</span>.</h1>
     <p class="ix-hero__lead">Fundamentals, tutorials, and how-tos for building
     safe, reliable and performant zero-copy systems with <code class="ix-hero__code">iceoryx2</code>.</p>
     <div class="ix-hero__cta">
       <a class="ix-btn ix-btn--primary" href="getting-started/quickstart.html">
         Get started
         <svg viewBox="0 0 24 24" width="17" height="17" aria-hidden="true"><path fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round" d="M5 12h14M13 6l6 6-6 6"/></svg>
       </a>
       <a class="ix-btn ix-btn--ghost" href="https://github.com/eclipse-iceoryx/iceoryx2" target="_blank" rel="noopener">
         <svg viewBox="0 0 24 24" width="17" height="17" aria-hidden="true"><path fill="currentColor" d="M12 .5A11.5 11.5 0 0 0 .5 12a11.5 11.5 0 0 0 7.86 10.92c.58.1.79-.25.79-.56v-2c-3.2.7-3.88-1.37-3.88-1.37-.53-1.34-1.3-1.7-1.3-1.7-1.06-.72.08-.71.08-.71 1.17.08 1.79 1.2 1.79 1.2 1.04 1.79 2.74 1.27 3.4.97.11-.76.41-1.27.74-1.56-2.55-.29-5.24-1.28-5.24-5.69 0-1.26.45-2.28 1.19-3.09-.12-.29-.52-1.46.11-3.05 0 0 .97-.31 3.18 1.18a11 11 0 0 1 5.8 0c2.2-1.49 3.17-1.18 3.17-1.18.63 1.59.23 2.76.11 3.05.74.81 1.19 1.83 1.19 3.09 0 4.42-2.69 5.39-5.25 5.68.42.36.8 1.08.8 2.18v3.23c0 .31.21.67.8.56A11.5 11.5 0 0 0 23.5 12 11.5 11.5 0 0 0 12 .5Z"/></svg>
         View on GitHub
       </a>
     </div>
   </div>

   <div class="ix-colophon">
     <a class="ix-colophon__item" href="https://projects.eclipse.org/projects/technology.iceoryx" target="_blank" rel="noopener" aria-label="An Eclipse Foundation project">
       <img class="ix-colophon__logo-img ix-colophon__logo-img--ef only-light" src="_static/eclipse-foundation-color.svg" alt="Eclipse Foundation" width="106" height="34" />
       <img class="ix-colophon__logo-img ix-colophon__logo-img--ef only-dark" src="_static/eclipse-foundation-white-orange.svg" alt="Eclipse Foundation" width="106" height="34" />
       <span class="ix-colophon__label">An Eclipse Foundation project</span>
     </a>

     <span class="ix-colophon__sep" aria-hidden="true"></span>

     <a class="ix-colophon__item" href="https://ekxide.io/" target="_blank" rel="noopener" aria-label="Built and maintained by ekxide">
       <span class="ix-colophon__label">Built &amp; maintained by</span>
       <img class="ix-colophon__logo-img only-light" src="_static/ekxide-light.svg" alt="ekxide" width="73" height="30" />
       <img class="ix-colophon__logo-img only-dark" src="_static/ekxide-dark.svg" alt="ekxide" width="73" height="30" />
     </a>
   </div>

.. toctree::
   :hidden:
   :maxdepth: 1
   :caption: User Manual

   introduction

.. toctree::
   :hidden:
   :maxdepth: 2

   getting-started/index

.. toctree::
   :hidden:
   :maxdepth: 2

   fundamentals/index

.. toctree::
   :hidden:
   :maxdepth: 2

   tutorials/index

.. toctree::
   :hidden:
   :maxdepth: 2

   how-to/index

.. toctree::
   :hidden:
   :maxdepth: 1

   glossary

.. toctree::
   :hidden:
   :maxdepth: 1
   :caption: API Reference

   Overview <https://eclipse-iceoryx.github.io/iceoryx2>
   Rust <https://docs.rs/iceoryx2/latest>
   Python <https://eclipse-iceoryx.github.io/iceoryx2/python/latest>
   C++ <https://eclipse-iceoryx.github.io/iceoryx2/cxx/latest>
   C <https://eclipse-iceoryx.github.io/iceoryx2/c/latest>
