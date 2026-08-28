The iceoryx2 Book
=================

.. meta::
   :description: The iceoryx2 Book — fundamentals, tutorials, and how-tos for building safe, reliable and performant zero-copy systems with iceoryx2.
   :property=og:description: The iceoryx2 Book — fundamentals, tutorials, and how-tos for building safe, reliable and performant zero-copy systems with iceoryx2.

.. raw:: html

   <div class="ix-hero">
     <div class="ix-hero__mark" aria-hidden="true">
       <svg viewBox="48 41.8 106.5 76.2" width="118" height="84" focusable="false">
         <g stroke="currentColor" stroke-width="0.6" stroke-linejoin="round">
           <path fill="#588bb5" d="M84.8672 96.2031 L76.9336 82.5977 L74.9492 102.582 Z"/>
           <path fill="#588bb5" d="M75.6563 81.0391 L71.8516 74.1602 L68.168 78.1289 Z"/>
           <path fill="#b3e6ef" d="M144.4531 76.9141 L130.3828 78.375 L110.5859 92.9063 Z"/>
           <path fill="#b3e6ef" d="M73.0078 100.1875 L73.7188 93.2461 L52.1016 113.7461 Z"/>
           <path fill="#588bb5" d="M144.5703 78.8359 L138.457 81.9844 L136.2656 95.5898 Z"/>
           <path fill="#b3e6ef" d="M 86.429688 94.785156 C 86.75 94.785156 101.570312 67.914062 101.570312 67.914062 L 73.390625 73.148438 L 86.429688 94.785156 "/>
           <path fill="#b3e6ef" d="M126.7734 74.7695 L131.5898 46.8984 L103.5 67.4063 Z"/>
           <path fill="#588bb5" d="M 92.820312 87.738281 L 106.863281 92.855469 L 128.308594 77.566406 L 102.914062 69.160156 L 92.820312 87.738281 "/>
           <path fill="#588bb5" d="M 131.261719 57.824219 L 150.371094 56.046875 L 150.371094 53.316406 L 132.828125 46.078125 L 131.261719 57.824219 "/>
           <path fill="#b3e6ef" d="M131.4531 59.3594 L136.5234 58.9727 L128.5273 75.5469 Z"/>
           <path fill="#588bb5" d="M129.7969 46.0977 L85.1406 46.0938 L124.3398 49.9961 Z"/>
         </g>
       </svg>
     </div>
     <p class="ix-hero__eyebrow"><span class="ix-hero__dot"></span>The <span class="ix-hero__word">iceoryx<span class="ix-hero__two">2</span></span> Book</p>
     <h1 class="ix-hero__title">The guide to iceoryx<span class="ix-hero__two">2</span>.</h1>
     <p class="ix-hero__lead">Fundamentals, tutorials, and how-tos for building
     safe, reliable and performant zero-copy systems with <code class="ix-hero__code">iceoryx2</code>.</p>
     <div class="ix-hero__cta">
       <a class="ix-btn ix-btn--ghost ix-btn--arrow" href="overview/index.html">
         Overview
         <svg viewBox="0 0 24 24" width="17" height="17" aria-hidden="true"><path fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round" d="M5 12h14M13 6l6 6-6 6"/></svg>
       </a>
       <a class="ix-btn ix-btn--ghost ix-btn--arrow" href="getting-started/quickstart.html">
         Get Started
         <svg viewBox="0 0 24 24" width="17" height="17" aria-hidden="true"><path fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round" d="M5 12h14M13 6l6 6-6 6"/></svg>
       </a>
       <a class="ix-btn ix-btn--ghost" href="https://github.com/eclipse-iceoryx/iceoryx2" target="_blank" rel="noopener">
         <svg viewBox="0 0 24 24" width="17" height="17" aria-hidden="true"><path fill="currentColor" d="M12 .5A11.5 11.5 0 0 0 .5 12a11.5 11.5 0 0 0 7.86 10.92c.58.1.79-.25.79-.56v-2c-3.2.7-3.88-1.37-3.88-1.37-.53-1.34-1.3-1.7-1.3-1.7-1.06-.72.08-.71.08-.71 1.17.08 1.79 1.2 1.79 1.2 1.04 1.79 2.74 1.27 3.4.97.11-.76.41-1.27.74-1.56-2.55-.29-5.24-1.28-5.24-5.69 0-1.26.45-2.28 1.19-3.09-.12-.29-.52-1.46.11-3.05 0 0 .97-.31 3.18 1.18a11 11 0 0 1 5.8 0c2.2-1.49 3.17-1.18 3.17-1.18.63 1.59.23 2.76.11 3.05.74.81 1.19 1.83 1.19 3.09 0 4.42-2.69 5.39-5.25 5.68.42.36.8 1.08.8 2.18v3.23c0 .31.21.67.8.56A11.5 11.5 0 0 0 23.5 12 11.5 11.5 0 0 0 12 .5Z"/></svg>
         View on GitHub
       </a>
       <a class="ix-btn ix-btn--ghost" href="https://community.iceoryx.io/" target="_blank" rel="noopener">
         <svg viewBox="0 0 36 37" width="17" height="17" aria-hidden="true">
           <path fill="currentColor" d="M17.784 0C8.129 0 0 8.122 0 18.144v18.788l17.781-.017c9.654 0 17.485-8.436 17.485-18.454C35.266 8.442 27.429 0 17.784 0Z"/>
           <path fill="#fff9ae" d="M17.966 7.023a10.73 10.73 0 0 0-9.309 5.493 11.35 11.35 0 0 0-.219 11.111l-1.961 6.546 7.042-1.651a10.57 10.57 0 0 0 11.955-2.139 11.38 11.38 0 0 0 2.552-12.31 10.73 10.73 0 0 0-10.049-7.051l-.011.001Z"/>
           <path fill="#00aeef" d="M26.441 25.228a10.46 10.46 0 0 1-12.922 3.275l-7.042 1.672 7.169-.879a10.53 10.53 0 0 0 14.143-2.954 11.38 11.38 0 0 0-1.225-14.918 11.37 11.37 0 0 1-.124 13.801l.001.003Z"/>
           <path fill="#00a94f" d="M25.834 23.093a10.56 10.56 0 0 1-12.521 4.657l-6.836 2.426 7.042-1.654a10.56 10.56 0 0 0 13.759-4.476 11.38 11.38 0 0 0-2.708-14.693 11.37 11.37 0 0 1 1.265 13.74Z"/>
           <path fill="#f15d22" d="M9.086 23.876a11.35 11.35 0 0 1 3.878-13.584 10.56 10.56 0 0 1 13.608 1.13A10.54 10.54 0 0 0 12.055 8.803a11.38 11.38 0 0 0-3.617 14.821l-1.961 6.546 2.609-6.294Z"/>
           <path fill="#d0232b" d="M8.438 23.627a11.35 11.35 0 0 1 2.499-13.911 10.55 10.55 0 0 1 13.632-.362A10.53 10.53 0 0 0 9.894 8.354a11.38 11.38 0 0 0-2.126 15.099l-1.289 6.724 1.959-6.55Z"/>
         </svg>
         Join the Community
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
   :maxdepth: 2
   :caption: User Manual

   overview/index

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
