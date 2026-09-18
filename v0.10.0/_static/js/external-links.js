/* Open external card links (`:link-type: url`) in a new tab; internal
   article links (`:link-type: doc`/`ref`) stay in the same tab. */
(function () {
  function mark() {
    document.querySelectorAll(".sd-card a.reference.external").forEach(function (a) {
      a.target = "_blank";
      a.rel = "noopener noreferrer";
    });
  }
  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", mark);
  } else {
    mark();
  }
})();
