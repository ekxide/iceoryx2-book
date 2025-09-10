The iceoryx2 Book
=================

.. image:: images/iceoryx-logo-01.png
   :class: hidden
   :alt: iceoryx logo

.. image:: images/iceoryx-logo-02.png
   :class: hidden
   :alt: iceoryx logo

.. image:: images/ekxide-black-no-frame.png
   :class: hidden
   :alt: ekxide logo

.. image:: images/ekxide-white-no-frame.png
   :class: hidden
   :alt: ekxide logo

.. raw:: html

   <style>
   .hidden { display: none; }
   
   .logo-container {
       position: relative;
       text-align: center;
       margin: 20px 0;
   }
   
   .logo-light, .logo-dark { 
       display: block;
       margin: 0 auto;
   }
   
   .ekxide-light, .ekxide-dark { 
       display: block; 
   }
   
   .logo-dark, .ekxide-dark { 
       display: none; 
   }
   
   @media (prefers-color-scheme: dark) {
       .logo-light, .ekxide-light { display: none; }
       .logo-dark, .ekxide-dark { display: block; }
   }
   
   html[data-theme="dark"] .logo-light,
   html[data-theme="dark"] .ekxide-light,
   [data-theme="dark"] .logo-light,
   [data-theme="dark"] .ekxide-light {
       display: none !important;
   }
   
   html[data-theme="dark"] .logo-dark,
   html[data-theme="dark"] .ekxide-dark,
   [data-theme="dark"] .logo-dark,
   [data-theme="dark"] .ekxide-dark {
       display: block !important;
   }
   
   html[data-theme="light"] .logo-light,
   html[data-theme="light"] .ekxide-light,
   [data-theme="light"] .logo-light,
   [data-theme="light"] .ekxide-light {
       display: block !important;
   }
   
   html[data-theme="light"] .logo-dark,
   html[data-theme="light"] .ekxide-dark,
   [data-theme="light"] .logo-dark,
   [data-theme="light"] .ekxide-dark {
       display: none !important;
   }
   </style>
   
   <div class="logo-container">
       <!-- Light theme logos -->
       <img src="_images/iceoryx-logo-01.png" 
            alt="iceoryx logo" 
            class="logo-light"
            style="max-width: 50%; height: auto;">
       <img src="_images/ekxide-black-no-frame.png" 
            alt="ekxide logo" 
            class="ekxide-light"
            style="position: absolute; 
                   bottom: 30px; 
                   right: 120px; 
                   width: 10%; 
                   height: auto;">
       
       <!-- Dark theme logos -->
       <img src="_images/iceoryx-logo-02.png" 
            alt="iceoryx logo" 
            class="logo-dark"
            style="max-width: 50%; height: auto;">
       <img src="_images/ekxide-white-no-frame.png" 
            alt="ekxide logo" 
            class="ekxide-dark"
            style="position: absolute; 
                   bottom: 30px; 
                   right: 120px; 
                   width: 10%; 
                   height: auto;">
   </div>

Welcome to the ``iceoryx2`` book! Brought to you with ♡ by the developers at
`ekxide IO <http://ekxide.io>`_.

.. toctree::
   :maxdepth: 1
   :caption: User Manual

   introduction

.. toctree::
   :maxdepth: 2

   getting-started/index

.. toctree::
   :maxdepth: 2

   fundamentals/index

.. toctree::
   :maxdepth: 1

   glossary

.. toctree::
   :maxdepth: 1
   :caption: API Reference

   Overview <https://eclipse-iceoryx.github.io/iceoryx2>
   Rust <https://docs.rs/iceoryx2/latest>
   Python <https://eclipse-iceoryx.github.io/iceoryx2/python/latest>
   C++ <https://eclipse-iceoryx.github.io/iceoryx2/cxx/latest>
   C <https://eclipse-iceoryx.github.io/iceoryx2/c/latest>
