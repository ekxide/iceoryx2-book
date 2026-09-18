/* Inject the fixed ambient glow (.ix-bg) behind all content. In JS to avoid
   overriding a Furo page template; styled in theme.css. */
(function () {
  function mount() {
    if (document.querySelector(".ix-bg")) return;
    var bg = document.createElement("div");
    bg.className = "ix-bg";
    bg.setAttribute("aria-hidden", "true");
    bg.innerHTML = '<div class="ix-bg__glow ix-bg__glow--1"></div>';
    document.body.insertBefore(bg, document.body.firstChild);
  }
  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", mount);
  } else {
    mount();
  }
})();
