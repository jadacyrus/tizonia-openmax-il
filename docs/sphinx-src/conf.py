# -*- coding: utf-8 -*-
#
# Tizonia documentation build configuration file, created by
# sphinx-quickstart on Sun Apr  8 23:37:03 2012.
#
# This file is execfile()d with the current directory set to its containing dir.
#
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

import sys
import os
import subprocess
import re
import alabaster
import zipfile
from collections import OrderedDict
from recommonmark.parser import CommonMarkParser

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
# sys.path.insert(0, os.path.abspath('.'))
#sys.path.append(os.path.abspath('/usr/local/lib/python2.7/dist-packages/breathe'))
#sys.path.append('/home/joni/work/3rdparty/breathe')

# -- General configuration -----------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be extensions
# coming with Sphinx (named 'sphinx.ext.*') or your custom ones.
extensions = ['sphinx.ext.todo', 'alabaster', 'breathe']

read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The suffix of source filenames.
source_parsers = {'.md': CommonMarkParser}
source_suffix = ['.rst', '.md']

# The encoding of source files.
#source_encoding = 'utf-8-sig'

# The master toctree document.
master_doc = 'index'

# General information about the project.
project = u'Tizonia'
copyright = u'2019, Aratelia Limited - Juan A. Rubio'

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = '0.17.0'
# The full version, including alpha/beta/rc tags.
release = '0.17.0'

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#language = None

# There are two options for replacing |today|: either, you set today to some
# non-false value, then it is used:
#today = ''
# Else, today_fmt is used as the format for a strftime call.
#today_fmt = '%B %d, %Y'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
exclude_patterns = ['_build']

# The reST default role (used for this markup: `text`) to use for all documents.
#default_role = None

# If true, '()' will be appended to :func: etc. cross-reference text.
#add_function_parentheses = True

# If true, the current module name will be prepended to all description
# unit titles (such as .. function::).
#add_module_names = True

# If true, sectionauthor and moduleauthor directives will be shown in the
# output. They are ignored by default.
#show_authors = False

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# A list of ignored prefixes for module index sorting.
#modindex_common_prefix = []

# Options for breathe extension
# -----------------------------

breathe_projects = {
    "tizonia":"../doxygen-src/xml/",
    }

breathe_default_project = "tizonia"

breathe_domain_by_extension = {
        "h" : "c",
        "c" : "c",
        "hpp" : "cpp",
        "cpp" : "cpp",
        "py": "py",
        }

# -- Options for HTML output ---------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
html_theme = 'alabaster'

extra_nav_links = OrderedDict()
extra_nav_links['Tizonia Website']  = 'http://www.tizonia.org'
extra_nav_links['Source (GitHub)'] = 'https://github.com/tizonia/tizonia-openmax-il'
extra_nav_links['Issues (GitHub)'] = 'https://github.com/tizonia/tizonia-openmax-il/issues'
extra_nav_links['Binaries (Bintray)']  = 'https://bintray.com/tizonia'

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#html_theme_options = {}
html_theme_options = {
    'logo': 'tizonia_logo_200.gif',
    'github_user': 'tizonia',
    'github_repo': 'tizonia-openmax-il',
    'github_type': 'star',
    'github_banner': True,
    'travis_button': 'true',
    'page_width': '90%',
    'extra_nav_links': extra_nav_links,
    'analytics_id': 'UA-21817639-6',
    'show_related': True,
    'fixed_sidebar': True
}

# Add any paths that contain custom themes here, relative to this directory.
#html_theme_path = []
html_theme_path = [alabaster.get_path()]

# The name for this set of Sphinx documents.  If None, it defaults to
# "<project> v<release> documentation".
#html_title = None

# A shorter title for the navigation bar.  Default is the same as html_title.
#html_short_title = None

# The name of an image file (relative to this directory) to place at the top
# of the sidebar.
#html_logo = None

# The name of an image file (within the static path) to use as favicon of the
# docs.  This file should be a Windows icon file (.ico) being 16x16 or 32x32
# pixels large.
#html_favicon = None

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

# If not '', a 'Last updated on:' timestamp is inserted at every page bottom,
# using the given strftime format.
#html_last_updated_fmt = '%b %d, %Y'

# If true, SmartyPants will be used to convert quotes and dashes to
# typographically correct entities.
#html_use_smartypants = True

# Custom sidebar templates, maps document names to template names.
#html_sidebars = {}
html_sidebars = {
    '**': [
        'about.html',
        'navigation.html',
        'relations.html',
        'searchbox.html',
    ]
}

# Additional templates that should be rendered to pages, maps page names to
# template names.
#html_additional_pages = {}

# If false, no module index is generated.
#html_domain_indices = True

# If false, no index is generated.
#html_use_index = True

# If true, the index is split into individual pages for each letter.
#html_split_index = False

# If true, links to the reST sources are added to the pages.
#html_show_sourcelink = True

# If true, "Created using Sphinx" is shown in the HTML footer. Default is True.
#html_show_sphinx = True

# If true, "(C) Copyright ..." is shown in the HTML footer. Default is True.
#html_show_copyright = True

# If true, an OpenSearch description file will be output, and all pages will
# contain a <link> tag referring to it.  The value of this option must be the
# base URL from which the finished HTML is served.
#html_use_opensearch = ''

# This is the file name suffix for HTML files (e.g. ".xhtml").
#html_file_suffix = None

# Output file base name for HTML help builder.
htmlhelp_basename = 'Tizoniadoc'


# -- Options for LaTeX output --------------------------------------------------

latex_elements = {
# The paper size ('letterpaper' or 'a4paper').
#'papersize': 'letterpaper',

# The font size ('10pt', '11pt' or '12pt').
#'pointsize': '10pt',

# Additional stuff for the LaTeX preamble.
#'preamble': '',
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title, author, documentclass [howto/manual]).
latex_documents = [
  ('index', 'Tizonia.tex', u'Tizonia Documentation',
   u'Juan A. Rubio', 'manual'),
]

# The name of an image file (relative to this directory) to place at the top of
# the title page.
#latex_logo = None

# For "manual" documents, if this is true, then toplevel headings are parts,
# not chapters.
#latex_use_parts = False

# If true, show page references after internal links.
#latex_show_pagerefs = False

# If true, show URL addresses after external links.
#latex_show_urls = False

# Documents to append as an appendix to all manuals.
#latex_appendices = []

# If false, no module index is generated.
#latex_domain_indices = True


# -- Options for manual page output --------------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    ('index', 'tizoniaopenmaxil', u'Tizonia Documentation',
     [u'Juan A. Rubio'], 1)
]

# If true, show URL addresses after external links.
#man_show_urls = False


# -- Options for Texinfo output ------------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
  ('index', 'Tizonia', u'Tizonia Documentation',
   u'Juan A. Rubio', 'Tizonia', 'One line description of project.',
   'Miscellaneous'),
]

# Documents to append as an appendix to all manuals.
#texinfo_appendices = []

# If false, no module index is generated.
#texinfo_domain_indices = True

# How to display URL addresses: 'footnote', 'no', or 'inline'.
#texinfo_show_urls = 'footnote'

def run_doxygen(app):
    """Run the doxygen make command in the docs/ top level folder"""

    try:
        repo_path = os.path.abspath("../../")
        cwd = os.getcwd()

        links = \
            [ \
              [ 'README.md', 'overview/README.md' ] , \
              [ 'PROJECT.md', 'overview/PROJECT.md' ] , \
              [ 'BUILDING.md', 'development/BUILDING.md' ] , \
            ]

        for link in links:
            source = os.path.join(repo_path, link[0])
            target = os.path.join(cwd, link[1])
            if not os.path.lexists(target):
                sys.stdout.write("Creating symlink : %s" % target)
                os.symlink(source, target)

        read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'
        if read_the_docs_build:
            retcode = subprocess.call("cd ../doxygen-src && doxygen doxyfile.rtd", shell=True)
        else:
            retcode = subprocess.call("cd .. && make", shell=True)
        if retcode < 0:
            sys.stderr.write("doxygen terminated by signal %s" % (-retcode))
    except OSError as e:
        sys.stderr.write("doxygen execution failed: %s" % e)


def setup(app):

    # Add hook for building doxygen xml when needed
    app.connect("builder-inited", run_doxygen)
