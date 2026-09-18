// Suppress CSS transitions for the instant the theme flips, so the sidebar
// links (and everything else) don't animate their color/background and flicker.
// Hover transitions are unaffected — the guard class is only on for ~2 frames.
(function () {
  var root = document.documentElement;

  function suppress() {
    root.classList.add("ix-theme-switching");
    requestAnimationFrame(function () {
      requestAnimationFrame(function () {
        root.classList.remove("ix-theme-switching");
      });
    });
  }

  // Capture phase: run before Furo's toggle handler flips the theme.
  document.addEventListener(
    "click",
    function (e) {
      if (e.target.closest && e.target.closest(".theme-toggle")) suppress();
    },
    true
  );

  // Auto mode: an OS color-scheme change also swaps the theme.
  if (window.matchMedia) {
    var mq = window.matchMedia("(prefers-color-scheme: dark)");
    if (mq.addEventListener) mq.addEventListener("change", suppress);
  }
})();
