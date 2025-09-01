# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'the iceoryx2 book'
copyright = '2025, ekxide.io'
author = 'ekxide developers'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.viewcode',
    'sphinxcontrib.mermaid',
    'myst_parser',
    'sphinx_design',
    "sphinx_multiversion",
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

templates_path = ['_templates']
exclude_patterns = []

source_suffix = {
    '.rst': 'restructuredtext',
    '.md': 'markdown',
}

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'
html_title = "the iceoryx2 book"
html_theme_options = {
}
html_static_path = ['_static']
html_sidebars = {
    "**": [
        "sidebar/scroll-start.html",
        "sidebar/brand.html",
        "sidebar/search.html",
        # TODO: Uncomment when versioning is implemented
        # 'version-picker.html',
        "sidebar/navigation.html",
        "sidebar/ethical-ads.html",
        "sidebar/scroll-end.html",
    ]
}
html_css_files = [
    'css/version-picker.css',
]
html_js_files = [
    'js/version-picker.js',
]
