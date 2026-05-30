# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'the iceoryx2 book'
copyright = '2025, ekxide.io'
author = 'ekxide developers'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.viewcode",
    "sphinxcontrib.mermaid",
    "myst_parser",
    "sphinx_design",
    "sphinx_multiversion",
    "sphinxext.opengraph",
]

myst_enable_extensions = [
    "deflist",
    "attrs_block",
    "attrs_inline",
]
myst_heading_anchors = 3

smv_branch_whitelist = r'^main$'
smv_tag_whitelist = r'^v\d+\.\d+\.\d+$'
smv_remote_whitelist = r'^origin$'
smv_released_pattern = r'^tags/v\d+\.\d+\.\d+$'

templates_path = ["_templates"]
exclude_patterns = []

source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}

# -- Options for linkcheck ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-the-linkcheck-builder

linkcheck_timeout = 30
linkcheck_workers = 5
linkcheck_retries = 3
linkcheck_ignore = [
    r'http://localhost.*',
    r'http://127\.0\.0\.1.*',
    r'.*ekxide\.io.*',
]

linkcheck_rate_limit_timeout = 30.0
linkcheck_anchors = True

import os
linkcheck_request_headers = {
    "*": {
        "User-Agent": "Mozilla/5.0 (X11; Linux x86_64; rv:100.0) Gecko/20100101 Firefox/100.0",
    },
    "github.com": {
        "Authorization": f"token {os.environ.get('GITHUB_TOKEN', '')}",
    },
}

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "furo"
html_title = "The iceoryx2 Book"
html_favicon = "_static/favicon.svg"
html_baseurl = "https://ekxide.github.io/iceoryx2-book/"

# -- Social previews (OpenGraph / Twitter) -----------------------------------
ogp_site_url = html_baseurl
ogp_site_name = "The iceoryx2 Book"
ogp_image = html_baseurl + "_static/og-card.png"
ogp_image_alt = "The iceoryx2 Book"
ogp_type = "website"
ogp_description_length = 200  # per-page description from the first paragraphs
ogp_enable_meta_description = True
ogp_custom_meta_tags = [
    '<meta name="twitter:card" content="summary_large_image">',
]

# Brand theme. The `--ix-*` keys become CSS custom properties for theme.css;
# defining them per light/dark here lets the custom layer re-theme across
# light/dark/auto without [data-theme] selectors.
html_theme_options = {
    "top_of_page_buttons": [],
    "sidebar_hide_name": True,  # the brand template renders the wordmark
    "light_css_variables": {
        # --- iceoryx brand tokens (light) ---
        # surfaces are slightly cool-tinted: neutral white reads warm next to
        # the cool page (simultaneous contrast).
        "ix-bg": "#f5f8fc",
        "ix-bg-2": "#e2eaf3",
        "ix-surface": "#fafcff",
        "ix-surface-2": "#f2f7fc",
        "ix-line": "rgba(15, 55, 90, 0.12)",
        "ix-line-soft": "rgba(15, 55, 90, 0.06)",
        "ix-ice": "#0a8aa0",
        "ix-ice-bright": "#0a93ab",
        "ix-ice-deep": "#0a7385",
        "ix-blue": "#3a66cf",
        "ix-mint": "#0e9b73",
        "ix-text": "#0c1825",
        "ix-muted": "#36475a",
        "ix-muted-2": "#586a80",
        "ix-glow-1": "rgba(10, 160, 185, 0.16)",
        "ix-glow": "0 0 40px rgba(10, 138, 160, 0.25)",
        # per-theme surfaces/effects kept as vars so the custom layer needs no
        # [data-theme] selectors (works under "auto")
        "ix-drawer-bg": "rgba(238, 243, 249, 0.72)",
        "ix-header-bg": "rgba(247, 250, 253, 0.82)",
        "ix-shadow": "0 16px 40px -30px rgba(20, 50, 90, 0.25)",
        "ix-card-bg": "#fafcff",
        "ix-card-shadow": "0 22px 44px -26px rgba(20, 50, 90, 0.28)",
        "ix-card-hover-border": "rgba(10, 138, 160, 0.40)",
        "ix-svg-panel": "transparent",
        "ix-svg-pad": "0",
        "ix-code-bg": "#eef3f9",
        "ix-code-hl": "rgba(10, 138, 160, 0.12)",
        "ix-selection-bg": "rgba(10, 138, 160, 0.22)",
        "ix-selection-fg": "#04222a",
        # --- Furo mappings (light) ---
        "color-brand-primary": "#0a8aa0",
        "color-brand-content": "#0a7d91",
        "color-background-primary": "#f5f8fc",
        "color-background-secondary": "#eef3f9",
        "color-background-hover": "#ffffff",
        "color-background-hover--transparent": "rgba(255,255,255,0)",
        "color-background-border": "rgba(15, 55, 90, 0.12)",
        "color-foreground-primary": "#0c1825",
        "color-foreground-secondary": "#46596b",
        "color-foreground-muted": "#586a80",
        "color-foreground-border": "rgba(15, 55, 90, 0.12)",
        "color-code-background": "#ffffff",
        "color-code-foreground": "#33485a",
        "color-link": "#0a7d91",
        "color-link--hover": "#0a93ab",
        "color-link-underline": "rgba(10, 138, 160, 0.30)",
        "color-link-underline--hover": "rgba(10, 138, 160, 0.7)",
        "color-sidebar-background": "#eef3f9",
        "color-sidebar-background-border": "rgba(15, 55, 90, 0.10)",
        "color-sidebar-item-background--hover": "rgba(10, 138, 160, 0.07)",
        "color-sidebar-link-text": "#46596b",
        "color-sidebar-link-text--top-level": "#0c1825",
        "color-sidebar-caption-text": "#586a80",
        "color-sidebar-search-border": "rgba(15, 55, 90, 0.12)",
        "color-toc-item-text": "#46596b",
        "color-toc-item-text--active": "#0a8aa0",
        "color-admonition-background": "#fafcff",
        "color-highlight-on-target": "rgba(10, 147, 171, 0.14)",
    },
    "dark_css_variables": {
        # --- iceoryx brand tokens (dark) ---
        "ix-bg": "#060910",
        "ix-bg-2": "#080d16",
        "ix-surface": "#141d2d",
        "ix-surface-2": "#1b2740",
        "ix-line": "rgba(150, 200, 230, 0.16)",
        "ix-line-soft": "rgba(150, 200, 230, 0.06)",
        "ix-ice": "#2dd4e8",
        "ix-ice-bright": "#7ff0ff",
        "ix-ice-deep": "#16b8d4",
        "ix-blue": "#5b8def",
        "ix-mint": "#6ff0c7",
        "ix-text": "#eaf3f7",
        "ix-muted": "#bccbd6",
        "ix-muted-2": "#6b7d8c",
        "ix-glow-1": "rgba(45, 212, 232, 0.30)",
        "ix-glow": "0 0 40px rgba(45, 212, 232, 0.35)",
        # surfaces & effects that differ per theme (see light comment)
        "ix-drawer-bg": "rgba(8, 13, 22, 0.60)",
        "ix-header-bg": "rgba(8, 13, 22, 0.82)",
        "ix-shadow": "0 18px 50px -34px rgba(0, 0, 0, 0.80)",
        "ix-card-bg": "linear-gradient(180deg, #18222f, #121b29)",
        "ix-card-shadow": "0 24px 50px -28px rgba(0, 0, 0, 0.85)",
        "ix-card-hover-border": "rgba(45, 212, 232, 0.40)",
        "ix-svg-panel": "#f7fbfd",
        "ix-svg-pad": "1rem",
        "ix-code-bg": "#0a111c",
        "ix-code-hl": "rgba(45, 212, 232, 0.12)",
        "ix-selection-bg": "rgba(45, 212, 232, 0.28)",
        "ix-selection-fg": "#ffffff",
        # --- Furo mappings (dark) ---
        "color-brand-primary": "#2dd4e8",
        "color-brand-content": "#2dd4e8",
        "color-background-primary": "#060910",
        "color-background-secondary": "#080d16",
        "color-background-hover": "#0c1320",
        "color-background-hover--transparent": "rgba(12,19,32,0)",
        "color-background-border": "rgba(150, 200, 230, 0.10)",
        "color-foreground-primary": "#eaf3f7",
        "color-foreground-secondary": "#9fb1bf",
        "color-foreground-muted": "#6b7d8c",
        "color-foreground-border": "rgba(150, 200, 230, 0.10)",
        "color-code-background": "#0a111c",
        "color-code-foreground": "#c5d4e0",
        "color-link": "#2dd4e8",
        "color-link--hover": "#7ff0ff",
        "color-link-underline": "rgba(45, 212, 232, 0.28)",
        "color-link-underline--hover": "rgba(127, 240, 255, 0.8)",
        "color-sidebar-background": "#080d16",
        "color-sidebar-background-border": "rgba(150, 200, 230, 0.08)",
        "color-sidebar-item-background--hover": "rgba(45, 212, 232, 0.07)",
        "color-sidebar-link-text": "#9fb1bf",
        "color-sidebar-link-text--top-level": "#eaf3f7",
        "color-sidebar-caption-text": "#6b7d8c",
        "color-sidebar-search-border": "rgba(150, 200, 230, 0.12)",
        "color-toc-item-text": "#9fb1bf",
        "color-toc-item-text--active": "#2dd4e8",
        "color-admonition-background": "#0c1320",
        "color-highlight-on-target": "rgba(45, 212, 232, 0.16)",
    },
}
html_static_path = ["_static"]
html_css_files = [
    "css/theme.css",
    "css/version-picker.css",
    "css/custom.css",
]
html_js_files = [
    "js/atmosphere.js",
    "js/version-picker.js",
    "js/external-links.js",
    "js/theme-transitions.js",
]


# -- inline-svg directive ----------------------------------------------------
# `{inline-svg} path` inlines an SVG file into the page (wrapped in a figure)
# so it reads the theme's --ix-* CSS variables and follows the light/dark
# toggle — which a plain <img> SVG cannot. Optional caption as directive body.
import re as _re
from pathlib import Path as _Path
from docutils import nodes as _nodes
from docutils.parsers.rst import Directive as _Directive, directives as _directives


class _InlineSvg(_Directive):
    required_arguments = 1
    final_argument_whitespace = True
    has_content = True
    option_spec = {"name": _directives.unchanged, "alt": _directives.unchanged}

    def run(self):
        env = self.state.document.settings.env
        arg = self.arguments[0].strip()
        if arg.startswith("/"):
            path = _Path(env.srcdir) / arg.lstrip("/")
        else:
            path = _Path(env.doc2path(env.docname)).parent / arg

        try:
            svg = path.read_text(encoding="utf-8")
        except OSError as exc:
            return [self.state_machine.reporter.error(
                f"inline-svg: cannot read {path}: {exc}", line=self.lineno)]
        env.note_dependency(str(path))

        # drop XML prolog / doctype so the markup inlines cleanly into HTML
        svg = _re.sub(r"<\?xml.*?\?>\s*", "", svg, flags=_re.DOTALL)
        svg = _re.sub(r"<!DOCTYPE.*?>\s*", "", svg, flags=_re.DOTALL)

        figure = _nodes.figure(classes=["ix-diagram"])
        figure += _nodes.raw("", svg, format="html")
        if self.content:
            caption = _nodes.caption()
            self.state.nested_parse(self.content, self.content_offset, caption)
            if len(caption) == 1 and isinstance(caption[0], _nodes.paragraph):
                caption.children = caption[0].children
            figure += caption
        self.add_name(figure)
        return [figure]


def setup(app):
    app.add_directive("inline-svg", _InlineSvg)
    return {"parallel_read_safe": True, "parallel_write_safe": True}
